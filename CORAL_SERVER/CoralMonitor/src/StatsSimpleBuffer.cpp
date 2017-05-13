#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <vector>

#include "CoralMonitor/StatsSimpleBuffer.h"
#include "CoralMonitor/StatsStorage.h"

namespace coral {

  bool simplefp_active = false;

  struct runDatPlotterData {

    std::string filename;

    std::string plotdir;

    size_t refresh;

    StatsSimpleBuffer* buffer;

  };

  static void*
  runDatPlotter(void* p)
  {
    //cast back
    runDatPlotterData* data = static_cast<runDatPlotterData*>(p);
    //find all desc of all registered stat-types
    while(simplefp_active)
    {

      //sleep(data->refresh);
      unsigned int iSlept = 0;
      while ( iSlept < data->refresh )
      {
        if ( simplefp_active ) sleep(1);
        iSlept++;
      }

      data->buffer->refresh();

      int fd = open(data->filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, (mode_t)0660 );
      if(fd > 0)
      {
        const std::list<std::string>& list = data->buffer->data();
        for( std::list<std::string>::const_iterator i = list.begin(); i != list.end(); ++i )
        {
          const std::string& line = *i;

          write(fd, line.c_str(), line.size());
          write(fd, "\n", 1);
        }
        close(fd);
      }

      //try to plot the gnuplot png

      const std::map<std::string, StatsSimpleBufferPlot*>& plots = data->buffer->plots();
      for( std::map<std::string, StatsSimpleBufferPlot*>::const_iterator i = plots.begin(); i != plots.end(); ++i )
      {
        StatsSimpleBufferPlot& plot(*(i->second));

        const std::vector< std::pair<std::string, size_t> >& columns = plot.gnuplot;

        if(columns.size() > 0)
        {
          std::ostringstream h;

          h << "set style data histeps";
          //add some gnuplot command to get a png
          h << "\nset terminal png nocrop size 640,480";
          h << "\nset title '" << plot.name << "'";
          if(!data->plotdir.empty())
            h << "\nset output '" << data->plotdir << '/' << plot.name << ".png'";
          else
            h << "\nset output '" << plot.name << ".png'";
          //now the plot sequence
          h << "\nplot ";

          std::vector< std::pair<std::string, size_t> >::const_iterator column = columns.begin();

          h << "\'" << data->filename << "\' using " << column->second << " title \'" << column->first << "\'";

          column++;

          for( ; column != columns.end(); ++column)
          {
            h << ", \'" << data->filename << "\' using " << column->second << " title \'" << column->first << "\'";

          }

          int fd2 = open("gnuplot.tmp", O_WRONLY | O_CREAT | O_TRUNC, (mode_t)0660 );
          if(fd2 > 0)
          {
            std::string line = h.str();

            write(fd2, line.c_str(), line.size());

            close(fd2);
          }

          system( "gnuplot gnuplot.tmp" );

        }
      }
    }


    delete data;

    pthread_exit(0);
  }


  StatsSimpleBuffer::StatsSimpleBuffer(size_t refresh_rate /* in seconds */, size_t maxsize)
    : m_refresh_rate( refresh_rate )
    , m_maxsize( maxsize )
    , m_last_refresh( time(0) )
  {
  }

  StatsSimpleBuffer::~StatsSimpleBuffer()
  {
    stopDatPlotter();
  }

  void
  StatsSimpleBuffer::refresh()
  {
    time_t current = time(0);
    time_t border = m_last_refresh  + m_refresh_rate;

    if( border <= current )
    {
      m_last_refresh = current;
      //create a small buffer
      std::vector<std::string> list;
      //fill up buffer with all current figures
      StatsStorageInstance().reportData(list);
      //merge all figures together
      std::ostringstream s;

      for( std::vector<std::string>::iterator i = list.begin(); i < list.end(); ++i )
      {
        s << *i << " ";
      }

      m_databuffer.push_back( s.str() );

      if(m_databuffer.size() > m_maxsize)
        m_databuffer.pop_front();
    }
  }

  void
  StatsSimpleBuffer::startDatPlotter(const std::string& filename, const std::string& plotdir, size_t refresh_rate)
  {
    pthread_attr_init(&m_dat_attr);
    pthread_attr_setdetachstate(&m_dat_attr, PTHREAD_CREATE_JOINABLE);

    simplefp_active = true;

    runDatPlotterData* data = new runDatPlotterData;
    //set default values or user settings
    if(filename.empty())
      data->filename = "monitor.dat";
    else
      data->filename = filename;

    data->plotdir = plotdir;

    if(refresh_rate > m_refresh_rate)
      data->refresh = refresh_rate;
    else
      data->refresh = m_refresh_rate;

    data->buffer = this;

    //check all
    std::vector<std::string> desc;
    StatsStorageInstance().reportDesc(desc);
    //print all plots available
    for(std::vector<std::string>::iterator i = desc.begin(); i != desc.end(); ++i )
    {
      std::cout << "Plot type : '" << *i << '\'' << std::endl;
    }
    //find all registered plots we want to plot
    for(std::map<std::string, StatsSimpleBufferPlot*>::iterator plot = m_plots.begin(); plot != m_plots.end(); ++plot )
    {
      StatsSimpleBufferPlot& rplot = *(plot->second);
      const std::vector<std::string>& stats(rplot.stats);

      for(size_t i = 0; i < desc.size(); ++i )
      {
        for(std::vector<std::string>::const_iterator j = stats.begin(); j != stats.end(); ++j )
        {
          if(j->compare(desc[i]) == 0)
            rplot.gnuplot.push_back(std::pair<std::string, size_t>(desc[i], i + 1) );
        }
      }
    }

    pthread_create(&m_dat_tid, &m_dat_attr, runDatPlotter, (void*)data);
  }

  void
  StatsSimpleBuffer::stopDatPlotter()
  {
    simplefp_active = false;

    void* status;
    pthread_join(m_dat_tid, &status);
  }

  StatsSimpleBufferPlot&
  StatsSimpleBuffer::addPlot(const std::string& name)
  {
    std::map<std::string, StatsSimpleBufferPlot*>::iterator i = m_plots.find(name);
    if(i != m_plots.end())
    {

      return *(i->second);
    }
    else
    {
      StatsSimpleBufferPlot* newplot = new StatsSimpleBufferPlot;

      m_plots.insert(std::pair<std::string, StatsSimpleBufferPlot*>(name, newplot));

      newplot->name = name;

      return *newplot;
    }
  }


}
