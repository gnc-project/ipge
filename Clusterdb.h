// Copyright Matt Wells, Jul 2002

// . a clusterRec now no longer exists, per se
// . it is the same thing as the key of the titleRec in titledb
// . titleRecs now contain the site and content hashes in the low bits
//   of their key. 
// . this allows us to store much cluster info in Titledb's RdbMap
// . so to get cluster info, just read in the titleRec, you do not even
//   need to uncompress it, just get the info from its key
// . we still use the cache here, however, to cache the keys (clusterRecs)
// . later, i may have to do some fancy footwork if we want to store all
//   clusterRecs (titleKeys) in memory.
// . TODO: what if stored file offsets in tfndb, too, then titledb RdbMap
//   would not be necessary?
//
// . clusterdb will now serve to help do fast site clustering by retaining
//   docids and site hashes in memory
//
//   00000000 00000000 0000000d dddddddd  d = docid
//   dddddddd dddddddd dddddddd dddddfll  f = family filter bit
//   llllssss ssssssss ssssssss sssssshz  q = year quarter bits
//                                        l = language bits
//   					  s = site hash
//   					  h = half bit
//   					  z = del bit

#ifndef _CLUSTERDB_H_
#define _CLUSTERDB_H_

//#include "TitleRec.h"  // SAMPLE_VECTOR_SIZE
#include "Rdb.h"
#include "Url.h"
#include "Conf.h"
#include "Titledb.h"
//#include "DiskPageCache.h"

// these are now just TitleRec keys
#define CLUSTER_REC_SIZE (sizeof(key_t))
// this now includes the gigabit vector
#define VECTOR_REC_SIZE (sizeof(key_t)+SAMPLE_VECTOR_SIZE+GIGABIT_VECTOR_SIZE)

class Clusterdb {

  public:

	// reset rdb
	void reset();
	
	// set up our private rdb
	bool init ( );

	// init the rebuild/secondary rdb, used by PageRepair.cpp
	bool init2 ( int32_t treeMem );

	bool verify ( char *coll );

	bool addColl ( char *coll, bool doVerify = true );

	Rdb *getRdb  ( ) { return &m_rdb; };

	// make the cluster rec
	void makeRecFromTitleRec ( char     *rec,
				   class TitleRec *titleRec,
				   bool      isDelKey );

	// make the cluster rec
	void makeRecFromTitleRecKey ( char *rec,
				      char *key,
				      bool  isDelKey );

	// make the cluster rec key
	key_t makeClusterRecKey ( int64_t     docId,
				  bool          familyFilter,
				  uint8_t       languageBits,
				  int32_t          siteHash,
				  bool          isDelKey,
				  bool          isHalfKey = false );

	key_t makeFirstClusterRecKey ( int64_t docId ) {
		return makeClusterRecKey ( docId, false, 0, 0, true ); };
	key_t makeLastClusterRecKey  ( int64_t docId ) {
		return makeClusterRecKey ( docId, true, 0xff, 0xffffffff,
					   false, true ); };

	// convert a titlerec key into a clusterec key
	key_t convertTitleRecKey ( key_t titleKey );

	/*
	uint32_t getGroupId ( int64_t docId ) {
		return g_titledb.getGroupId ( docId ); };
		
	// cluster rec should be stored on same host as titleRec with the
	// same docId that this key contains
	uint32_t getGroupIdFromKey ( key_t *key ) {
		return g_titledb.getGroupId ( getDocId ( *key ) ); };
	*/

	// NOTE: THESE NOW USE THE REAL CLUSTERDB REC
	// // docId occupies the most significant bytes of the key
	// now docId occupies the bits after the first 23
	int64_t getDocId ( void *k ) {
		//int64_t docId = (k.n0) >> (32+24);
		//docId |= ( ((uint64_t)(k.n1)) << 8 );
		int64_t docId = (((key_t *)k)->n0) >> 35;
		docId |= ( ((uint64_t)(((key_t *)k)->n1)) << 29 );
		return docId;
	};

	//int64_t getDocId ( char *r ) {
	//	return getDocId(*(key_t*)r);
	//}

	uint32_t getSiteHash26 ( char *r ) {
		//return g_titledb.getSiteHash ( (key_t *)r ); };
		return ((uint32_t)(((key_t*)r)->n0 >> 2) & 0x03FFFFFF);
	};

	uint32_t hasAdultContent ( char *r ) {
		//return g_titledb.hasAdultContent ( *(key_t *)r ); };
		return ((uint32_t)(((key_t*)r)->n0 >> 34) & 0x00000001);
	};

	unsigned char getLanguage ( char *r ) {
		return ((unsigned char)(((key_t*)r)->n0 >> 28) & 0x0000003F);
	}

	// NOTE: THESE USE THE OLD "CLUSTERDB" REC GENERATED BY MSG22 (VECTOR)
	//uint32_t getContentHash ( char *r ) {
	//	return g_titledb.getContentHash ( *(key_t *)r ); };

	char getFamilyFilter ( char *r ) {
		if ( (*(int64_t *)r) & 0x0000000400000000LL ) return 1;
		return 0;
	};


	//uint32_t hasAdultWords   ( char *r ) {
	//	return g_titledb.hasAdultWords ( *(key_t *)r ); };

	//uint32_t hasAdultCategory ( char *r ) {
	//	return g_titledb.hasAdultCategory ( *(key_t *)r ); };

	//unsigned char getLanguageFromVector ( char *r ) {
	//	return 0;
	//}

	// the random sample vector
	/*
	void getSampleVector     ( char *vec  , 
				   class Doc *doc, 
				   char *coll ,
				   int32_t  collLen ,
				   int32_t niceness = 0 );
	*/
	//void getSampleVector     ( char *vec  , class TermTable *table );
	char getSampleSimilarity ( char *vec0 , char *vec1 , int32_t size );
	// get the content vector from a cluster rec (used by Msg38.cpp)
	//char *getSampleVector  ( char *rec ) { return rec + sizeof(key_t); };

	//char *getGigabitVector    ( char *rec ) { 
	//	return rec + sizeof(key_t) + SAMPLE_VECTOR_SIZE ; };
	//char getGigabitSimilarity ( char *vec0 , char *vec1 ,
	//			    int32_t *qtable , int32_t numSlots ) ;

	//DiskPageCache *getDiskPageCache() { return &m_pc; };

  private:

	// this rdb holds urls waiting to be spidered or being spidered
	Rdb m_rdb;

	//DiskPageCache m_pc;
};

extern class Clusterdb g_clusterdb;
extern class Clusterdb g_clusterdb2;

#endif