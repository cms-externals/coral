#include <cerrno>
#include <cstdio> // For sprintf on gcc46 (bug #89762)
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "CoralBase/MessageStream.h"

#include "CoralMonitor/StatsCSVPlotter.h"
#include "CoralMonitor/StatsStorage.h"

#define LOG_INFO( msg ){ MessageStream myMsg("StatsCSVPlotter"); myMsg << Info << msg << coral::MessageStream::endmsg; }
#define LOG_WARNING( msg ){ MessageStream myMsg("StatsCSVPlotter"); myMsg << Warning << msg << coral::MessageStream::endmsg; }

namespace coral {

  static bool csvplotter_active = false;

  struct StatsCSVPlotterData {

    size_t refresh;

    int rotate;

    std::string fname;

    std::string fextension;

  };

  static void
  getHostnameLookup(char* buffer, size_t size)
  {
    char hostname[201];

    ::gethostname(hostname, 200);

    struct addrinfo hints;
    struct addrinfo* addr;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_flags = AI_CANONNAME;

    int res = getaddrinfo(hostname, 0, &hints, &addr);

    struct addrinfo* p;
    if(res == 0)
    {
      bool noset = true;
      for(p = addr; p != NULL; p = p->ai_next)
      {
        if(p->ai_canonname != 0)
        {
          strncpy(buffer, p->ai_canonname, size);
          noset = false;
        }
      }
      freeaddrinfo(addr);
      if(noset)
      {
        LOG_WARNING("Can't get canonname from addrinfo" );
        strncpy(buffer, hostname, size);
      }
    }
    else
    {
      LOG_WARNING("Can't resolve hostname : " << gai_strerror(res) )
        strncpy(buffer, hostname, size);
    }
  }

  void
  appendFileAttributes(std::string& s)
  {
    // Static Buffer
    char buffer[350];
    // Set the first char to _
    buffer[0] = '_';
    // Fill buffer with the hostname
    getHostnameLookup(buffer + 1, 300);
    // Append to filename
    s.append( buffer );
    // Fill buffer with timestamp
    snprintf(buffer + 1, 300, "%llu", (unsigned long long)time(0));
    // Append to filename
    s.append( buffer );
  }

  void
  runCSVPlotter_writeHeader(const char* filename, const std::vector<std::string>& list)
  {
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, (mode_t)0660 );
    if(fd > 0)
    {
      write(fd, "data,time,", 10);

      for( std::vector<std::string>::const_iterator i = list.begin(); i != list.end(); ++i )
      {
        write(fd, i->c_str(), i->size());
        write(fd, ",", 1);
      }
      write(fd, "\n", 1);
      close(fd);
    }
    else
      // Log only as Warning, instead of exception
      LOG_WARNING( "Can't open CSV-File '" << filename << "' for writing: " << strerror(errno) )
        }

  void
  runCSVPlotter_writeData(const char* filename, const std::vector<std::string>& list)
  {
    int fd = open(filename, O_WRONLY | O_APPEND, (mode_t)0660 );
    if(fd > 0)
    {
      size_t llen = list.size();

      time_t t01 = time(0);
      // Transform the time to human readable
      tm* l01 = localtime( &t01 );
      char b01[25];

      sprintf(b01, "%d-%02d-%02d,%02d:%02d:%02d",
              (l01->tm_year + 1900), (l01->tm_mon + 1),
              l01->tm_mday, l01->tm_hour, l01->tm_min, l01->tm_sec);

      write(fd, b01, 19);
      write(fd, ",", 1);

      for( size_t i = 0; i < llen; ++i )
      {
        write(fd, list[i].c_str(), list[i].size() );
        write(fd, ",", 1);
      }
      write(fd, "\n", 1);
      close(fd);
    }
    else
      // Log only as Warning, instead of exception
      LOG_WARNING( "Can't open CSV-File '" << filename << "' for writing: " << strerror(errno) )
        }

  static void*
  runCSVPlotter(void* p)
  {
    StatsCSVPlotterData* data = static_cast<StatsCSVPlotterData*>(p);

    std::string filename;
    if(data->rotate >= 0)
      // Log rotate or Rename was activated
    {
      filename.append(data->fname);
      appendFileAttributes( filename );
      filename.append(data->fextension);
    }
    else
    {
      filename.append(data->fname);
      filename.append(data->fextension);
    }

    std::vector<std::string> dlist;
    StatsStorageInstance().reportDesc(dlist);

    runCSVPlotter_writeHeader(filename.c_str(), dlist);

    time_t lastrotate = time(0);

    while(csvplotter_active)
    {

      //sleep(data->refresh);
      unsigned int iSlept = 0;
      while ( iSlept < data->refresh )
      {
        if (csvplotter_active) sleep(1);
        iSlept++;
      }

      if(data->rotate > 0)
      {
        time_t timeout = lastrotate + data->rotate;
        time_t current = time(0);

        if( timeout < current )
          // Do a log rotate
        {
          LOG_INFO( "Perform a log rotate" )
            // Rename the current log file
            filename.clear();

          filename.append(data->fname);
          appendFileAttributes( filename );
          filename.append(data->fextension);

          runCSVPlotter_writeHeader(filename.c_str(), dlist);

          lastrotate = current;
        }
      }

      std::vector<std::string> list;
      StatsStorageInstance().reportData(list);

      runCSVPlotter_writeData(filename.c_str(), list);
    }

    delete data;

    pthread_exit(0);
  }

  StatsCSVPlotter::StatsCSVPlotter(size_t refresh_rate, const std::string& filename, int rotate_rate)
  {
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    csvplotter_active = true;

    StatsCSVPlotterData* data = new StatsCSVPlotterData;

    data->refresh = refresh_rate;
    data->rotate = rotate_rate;

    // Set the correct filename
    // We need to split the filename in name and extension
    size_t fpos = filename.find_last_of('.');
    if(fpos == std::string::npos)
      // No '.' found in the prefered filename
    {
      data->fname.append( filename );
    }
    else
    {
      data->fname = filename.substr(0, fpos);
      data->fextension = filename.substr(fpos);
    }

    pthread_create(&m_tid, &attr, runCSVPlotter, (void*)data );
  }

  StatsCSVPlotter::~StatsCSVPlotter()
  {
    csvplotter_active = false;

    void* status;
    pthread_join(m_tid, &status);
  }

}
