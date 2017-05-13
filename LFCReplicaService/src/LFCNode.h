#ifndef LFCREPLICASERVICE_LFCNODE_H
#define LFCREPLICASERVICE_LFCNODE_H

#include <string>
#include <vector>
#include <map>

#include "LFCReplica.h"

namespace coral {

  namespace LFCReplicaService {

    class LFCSession;

    /**
     *  @class LFCNode
     *  @author Alexander Kalkhof
     *
     *  This class is a virtual presentation of each LFC node
     *
     *  The content is cached
     *  real LFC calls are invoked to fill the cache
     *
     *  each node is a file or folder
     */
    class LFCNode
    {

    public:

      /**
       * Constructor for none validated node
       */
      LFCNode( const std::string& path, const std::string& name );

      /**
       * Constructor for a validated node
       */
      LFCNode( const std::string& path, const std::string& name, bool isDir, bool readOk, bool writeOk, bool exeOk, const char* guid );

      /**
       * Destructor
       */
      ~LFCNode();

      /**
       * Returns the name of the node
       */
      const std::string& name() const { return m_name; };

      /**
       * Returns the path of the node
       */
      const std::string& path() const { return m_path; };

      /**
       * Returns the name as absolute path
       */
      std::string absolutePath();

      /**
       * Is the node valid/exists in the LFC
       *
       * This call invokes a real LFC lookup if not cached
       */
      bool valid();

      /**
       * Returns DIR flag
       */
      bool isDir() const { return m_isdir; };

      /**
       * Returns READABLE flag
       */
      bool isReadable() const { return m_readOk || m_writeOk; };

      /**
       * Returns WRITEABLE flag
       */
      bool isWritable() const { return m_writeOk; };

      /**
       * Returns EXECUTION flag
       */
      bool isExecutable() const { return m_executeOk; };

      /**
       * Returns global unique identifier
       */
      const std::string guid() const { return m_guid; };

      /**
       * Manually add a replica
       *
       * WARNING: replica will be added to LFC
       */
      LFCReplica* newReplica(const ReplicaDescription&);

      /**
       * Manually add a replica
       *
       * WARNING: folder will be added to LFC
       */
      LFCNode* newNode( const std::string& name, bool isDir, bool isWritable, bool isExecutable );

      /**
       * Returns all replicas
       *
       * WARNING: this call does not lookup the LFC
       * call another method before
       */
      const std::vector<LFCReplica*>& getReplicas();

      /**
       * WARNING: this call removes ALL REPLICAS in the LFC
       */
      void clearReplicas();

      /**
       * Returns the content
       *
       * This call invokes a real LFC lookup if not cached
       */
      const std::map<std::string, LFCNode*>& getContent();

      /**
       * Returns if the file exists in the folder node
       *
       * This call invokes a real LFC lookup if not cached
       */
      LFCNode* hasNode( const std::string& name );

      /**
       * Adds a new node to the content
       *
       * WARNING: use this method only if the node exists in the LFC
       * Further errors will happen if the node not exists
       */
      LFCNode* hasNode( const std::string& name, bool isDir, bool readOk, bool writeOk, bool exeOk );

      void setPermissionToGroup(const std::string& groupName, int mode);

      int getPermissionToGroup(const std::string& groupName);

      static void validatePath( std::string& path, std::string& name );

    private:

      void appendReplica( LFCReplica* );

      void lookupSelf();

      void lookupContent();

      void lookupReplicas();

    private:

      std::string m_name;

      std::string m_path;

      bool m_lfclookup_self;

      bool m_lfclookup_content;

      bool m_lfclookup_replicas;

      bool m_valid;

      bool m_isdir;

      bool m_readOk;

      bool m_writeOk;

      bool m_executeOk;

      std::string m_guid;

      std::map<std::string, LFCNode*> m_content;

      std::vector<LFCReplica*> m_replicas;

    };

  }

}

#endif
