/**
 *	@Copyright (c) 2016 joni <joni.kartorz.lee@gmail.com>
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <unistd.h>
#include <errno.h>
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

#include "IndexTreeWriter.h"

IndexTreeWriter::IndexTreeWriter(int dataItemSize, bool duplicateIndex, bool online):
m_duplicateIndexFlag(false),
m_totalChrindex(0),
m_strinxWordsMax(STRINX_WORDS_MAX),
m_strinxDepthMax(STRINX_DEPTH_MAX),
m_bOnLineRW(online),
m_bDuplicateIndex(duplicateIndex)
{
    m_dataItemSize = dataItemSize;
}

void IndexTreeWriter::open()
{
#ifdef _LINUX
    m_inxFile = fopen("/tmp/inxtree_tmp", "w+");
    m_dataTmpFile = fopen("/tmp/inxtree_data_tmp", "w+");
#elif defined(WIN32)
    m_inxFile = fopen("inxtree_tmp", "w+bTD");
#endif
    m_inxDataLen = 0;
    m_dataLoc = 1;
    m_strIndexLoc = 1;
    m_chrIndexLoc = 1;
    m_totalEntry = 0;

    struct inxtree_chrindex charIndex;
    memset(&charIndex, 0, sizeof(struct inxtree_chrindex));
    m_indexTree = new  ktree::kary_tree<inxtree_chrindex>(charIndex, 1);
}

bool IndexTreeWriter::load(const string& inxFilePath, int magic, bool online)
{
    m_bOnLineRW = online;
    m_inxFilePath = inxFilePath;
    if (IndexTree::load(inxFilePath, magic, false)) {
        fseek(m_inxFile, 0L, SEEK_END);
        m_inxDataLen = ftell(m_inxFile) - (m_dataLoc-1)*INXTREE_BLOCK;
        m_dataTmpFile = fopen("/tmp/inxtree_data_tmp", "w+");
        //m_outputPath = inxFilePath + "_tmp";
        //printf("load done: %s base loc 0x%x, dataloc:%d, 0x%x\n",inxFilePath.c_str(), m_inxDataLen, m_dataLoc, ftell(m_inxFile));
        return true;
    }

    open();
   // m_outputPath += "_tmp";
    //fseek(m_inxFile, 0L, SEEK_END); //  Opening a file in append mode will do this.
    return false;
}

IndexTreeWriter::~IndexTreeWriter()
{
    sync();
}

void IndexTreeWriter::clear()
{
    if (m_indexTree)
        delete m_indexTree;

    if (m_inxFile != NULL)
        fclose(m_inxFile);

    if (m_dataTmpFile != NULL)
        fclose(m_dataTmpFile);

    open();
}

void IndexTreeWriter::setStrinxThreshold(int wordsMax, int depthMax)
{
    m_strinxWordsMax = wordsMax;
    m_strinxDepthMax = depthMax;
}

bool IndexTreeWriter::add(u32 *key, int keylen, void *dataPtr, int dataLen)
{
    indextree::MutexLock lock(m_cs);

    fseek(m_dataTmpFile, 0L, SEEK_END);
    off_t start = m_inxDataLen + ftello(m_dataTmpFile);
    if (m_dataItemSize == 0) {
        unsigned char len[2];
        inxtree_write_u16(len, dataLen);
        fwrite((void *)len, 2, 1, m_dataTmpFile);
    }
    fwrite(dataPtr, dataLen, 1, m_dataTmpFile);
    //printf("IndexTreeWriter::add, base:0x%x loc 0x%x, dataLen %d\n", m_inxDataLen, start, dataLen);
    addToIndextree(m_indexTree->root(), start, key, key + keylen);
    return true;
}

void IndexTreeWriter::add(string key)
{
    u32* keyPtr = NULL;
    int keylen = IndexTreeHelper::utf8StrToUcs4Str(key.c_str(), &keyPtr);
    if (keylen > 0) {
        addToIndextree(m_indexTree->root(), 0, keyPtr, keyPtr + keylen);
        free(keyPtr);
    }
}

void IndexTreeWriter::addToIndextree(ktree::tree_node<inxtree_chrindex>::treeNodePtr parent,
                                     const off_t d_off, u32 *keyStartPtr, u32 *keyEndPtr)
{
    int i = 0;
	const u32 key = *(keyStartPtr++);
	if (!key)
	    return;

    static int cache[3] = {-1, -1, -1};
    static int cchinx = 0;

	ktree::tree_node<inxtree_chrindex>::treeNodePtr next;
	int size = parent->size();
	bool found = false;
    u32 chr;
    int pos=0;

    //printf("1: %u, %d\n", get_timems(), size);
    if (size > 0) {
        if (cchinx < 3) {
            if (cache[cchinx] != -1) {
                pos = cache[cchinx];
                chr = inxtree_read_u32(parent->child(pos)->value().wchr);
                if (chr == key) {
                    found = true;
                    goto ADD;
                }
                // a new tree, clear cache.
                for (int i=cchinx; i<3; i++)
                    cache[i] = -1;
            }
        }

        pos = size - 1;
        chr = inxtree_read_u32(parent->child(pos)->value().wchr);
        if (chr == key) {
            found = true;
            goto ADD;
        } else if(chr < key) {
            pos = size;
            goto ADD;
        }

        pos = bsearch(parent, key, 0, size-1);
        //printf("bsearch done: pos(%u)\n", pos);
        chr = inxtree_read_u32(parent->child(pos)->value().wchr);
        if (chr == key) {
            found = true;
            goto ADD;
        }
        if (chr < key && pos < size) {
            ++pos;
            //printf("new pos(%u)\n", pos);
        }
        goto ADD;
    }
    //printf("2: %u\n", get_timems());
ADD:
	bool leaf = keyStartPtr == keyEndPtr ? true : false;

    struct inxtree_chrindex  charInx;
    inxtree_write_u32(charInx.wchr, key);
    inxtree_write_u16(charInx.len_content, 0);
    if (leaf) {
	    inxtree_write_u32(charInx.location, d_off);
        ++m_totalEntry;
    } else {
		inxtree_write_u32(charInx.location, INXTREE_INVALID_ADDR);
    }

    if (!found) {
        if (pos == size)
    	    next = parent->insert(charInx);
        else
    		next = parent->insert(charInx, pos);
    } else {
        if (leaf) {
            if (inxtree_read_u32((*parent)[pos]->value().location) == INXTREE_INVALID_ADDR) {
                inxtree_write_u32((*parent)[pos]->value().location, d_off);
            } else {
                if (m_bDuplicateIndex) {
                    printf("WARRNING: append a duplicate index--last u32('%lc')\n", chr);
                    m_duplicateIndexFlag = true;
                    inxtree_write_u32(charInx.wchr, 0);
                    (*parent)[pos]->insert(charInx, 0);
                } else {
                    printf("WARRNING: ignore a duplicate index--last u32('%lc')\n", chr);
                }
            }
        } else {
            next = (*parent)[pos];
        }
    }
    if (cchinx < 3) {
        cache[cchinx] = pos;
    }
    /* advance to next level */
	if (!leaf) {
        ++cchinx;
		addToIndextree(next, d_off, keyStartPtr, keyEndPtr);
        --cchinx;
    }
}

bool IndexTreeWriter::write(string output)
{
    indextree::MutexLock lock(m_cs);

#ifdef _LINUX
	FILE *strinxTmpFile= fopen("/tmp/inxtree_tmp2", "w+");
#elif defined(WIN32)
	FILE *strinxTmpFile = fopen("inxtree_tmp2", "w+bTD");
#endif
    if (output == "") {
        output = "/tmp/inxtreewriter_output_tmp";
    }
    printf("write %s\n", output.c_str());

    FILE *outputFile = fopen(output.c_str(),"w+b");
    if (!outputFile) {
        printf("can't open %s, errno:%d\n", output.c_str(), errno);
        return false;
    }

    if (!m_bOnLineRW)
        trimIndexTree(m_indexTree->root(), 0, strinxTmpFile);

	/*- Write char index to dict file. */
    fseek(outputFile, (INDEX_BLOCK_NR-1)*INXTREE_BLOCK, SEEK_SET);
    	// Write root node.
	ktree::tree_node<inxtree_chrindex>::treeNodePtr rootNode = m_indexTree->root();
	struct inxtree_chrindex& rootIndex = rootNode->value();
	inxtree_write_u32(rootIndex.location, sizeof(struct inxtree_chrindex));
	inxtree_write_u16(rootIndex.len_content, rootNode->size());
	fwrite(&rootIndex, sizeof(struct inxtree_chrindex), 1, outputFile);
	m_totalChrindex = 1;
	// Write all nodes recursively.
	writeCharIndex(rootNode, outputFile);

    printf("write total %d, \n", m_totalChrindex);
	/*- Merge temple files */
	m_header.loc_chrindex[0] = INDEX_BLOCK_NR;
    inxtree_write_u32(m_header.d_entries, m_totalEntry);

	fseek(outputFile, 0, SEEK_END);
	int bnr = INXTREE_BLOCK_NR(ftello(outputFile)) + 1;
	inxtree_write_u32(m_header.loc_strindex, bnr);
    m_header.flags[0] |= m_duplicateIndexFlag ? F_DUPLICATEINX : 0;

	fseek(outputFile, (bnr-1)*INXTREE_BLOCK, SEEK_SET);
	fseek(strinxTmpFile, 0, SEEK_SET);
	IndexTreeHelper::mergeFile(outputFile, strinxTmpFile);

	bnr = INXTREE_BLOCK_NR(ftello(outputFile)) + 1;
	inxtree_write_u32(m_header.loc_data, bnr);
	fseek(outputFile, (bnr-1)*INXTREE_BLOCK, SEEK_SET);

	fseek(m_inxFile, (m_dataLoc-1)*INXTREE_BLOCK, SEEK_SET);
	IndexTreeHelper::mergeFile(outputFile, m_inxFile);
	fseek(m_dataTmpFile, 0, SEEK_SET);
    IndexTreeHelper::mergeFile(outputFile, m_dataTmpFile);

	fseek(outputFile, 0, SEEK_SET);
	fwrite(&m_header, sizeof(struct inxtree_header), 1, outputFile);


	fclose(outputFile);
	fclose(strinxTmpFile);
    fclose(m_inxFile);
    m_inxFile = NULL;
    fclose(m_dataTmpFile);
    strinxTmpFile = NULL;

    if (m_bOnLineRW) {
        try {
            copy_file(output, m_inxFilePath, copy_option::overwrite_if_exists);
        } catch (const filesystem_error& ex) {
            printf("%s", ex.what());
        }

        load(m_inxFilePath, 0xB4B3, true);
        // for add other words.
        //fseek(m_inxFile, 0L, SEEK_END);
    } else {
        printf("write dict file, done: \n");
        printf("    entries: %d\n", m_totalEntry);
        printf("    char index: %d\n", m_totalChrindex);
        //printf("    costs: (%u)s\n", Util::getTimeMS()/1000);
    }
}

int IndexTreeWriter::bsearch(ktree::tree_node<inxtree_chrindex>::treeNodePtr parent,
                             u32 key, int min, int max)
{
    int mid = (min + max) / 2;
    u32 chr = inxtree_read_u32(parent->child(mid)->value().wchr);
    //printf("bsearch min(%d), mid(%d), max(%d): chr(%u)-->key(%u)\n", min, mid, max, chr, key);
    if (min >= max) {
        return min;
    }
	if (chr == key)
        return mid;
    if (chr < key)
	    return bsearch(parent, key, mid+1, max);

    if (mid > min)
        return bsearch(parent, key, min, mid-1);
    else
        return min;
}

/*
 *  Strip string index, Save it to sinxfile.
 *
 *  'parent' should be saved to char index area, check if its children should be saved to 
 *  char index area or string index area. If a child should be saved to char index area,
 *  then all the children should be save to char index area.
 *
 *  'parent' can't be a index-- location must be INXTREE_INVALID_ADDR.
 *
 *  "isInStringIndex(..)" decides which index area children should be saved to.
 *
 */
void IndexTreeWriter::trimIndexTree(ktree::tree_node<inxtree_chrindex>::treeNodePtr parent,
                                     int depth, FILE* sinxfile)
{
	bool bIsStrIndex = false;
	if (++depth > CHRINX_DEPTH_MIN &&
	    inxtree_read_u32(parent->value().location) == INXTREE_INVALID_ADDR) {
		bIsStrIndex = isInStringIndex(parent, 0, 0);
	}
	if (bIsStrIndex) {
		struct inxtree_chrindex& cinx = parent->value();
		//inxtree_write_u32(cinx.location, ftello(sinxfile));
		int len_content = 0;
		off_t loc = 0;
        writeStringIndex(parent, 0, &len_content, &loc, sinxfile);/* parent is char index */
        inxtree_write_u32(cinx.location, loc);
        cinx.location[3] |= 0x80;
		inxtree_write_u16(cinx.len_content, len_content);
		parent->clear(); /* Children have been save to string index area */
	} else {
		//parent.len_content = parent->children().size();
		for (int i=0; i<parent->size(); i++) {
			trimIndexTree((*parent)[i], depth, sinxfile);
		}
	}
}

/// The below three functions "is_in_stringindex", "write_stringindex" and "trim_indextree"
/// strip string index from index tree and write it to a temp file.
/// The rule for stripping string index is in the function is_in_stringindex.

/* Check if paren's children should be recorded in string index area.
 * If meet the following conditions:x
 *    - the number of words less equal than STRINX_WORDS_MAX.
 *    - the length of tree less equal than STRINX_DEPTH_MAX.
 *
 * Figures:
 *    "&" represents a end node(location != INXTREE_INVALID_ADDR).
 *    "@" represents a normal node.
 *
 *             @             @          @    -- (parent, 0, 0)
 *           / |          /  |   \      |
 *          &  @         &   @ .. @     @
 *         /   |        /    |  |       |
 *        &    &       &     &  $       ..
 *                                      |
 *                                      @
 *                                      |
 *                                      &
 *
 *         figure-1    figure-2     figure-3
 *
 *    figure-1 and figure-3 should be string index.
 *    figure-2 should be char index.
 */
bool IndexTreeWriter::isInStringIndex(ktree::tree_node<inxtree_chrindex>::treeNodePtr parent,
                                      int words, int depth)
{
    if (depth == 0 && parent->size() == 0)
        return false;

	if (parent->size() > m_strinxWordsMax)
		return false;
	// firgure-3, if there is one more children, this node should be char index.
	int max_depth = words > 1 ? m_strinxDepthMax : STRINX_LEN_MAX;
	if (++depth > max_depth)
		return false;

	for (int i = 0; i < parent->size(); i++) {
		struct inxtree_chrindex& charIndex = (*parent)[i]->value();
		if (inxtree_read_u32(charIndex.location) != INXTREE_INVALID_ADDR) {
			if (++words > m_strinxWordsMax)
				return false;
		}
		if (!isInStringIndex((*parent)[i], words, depth))
			return false;
	}
	return true;
}

/* @len_inx - length of string.
   @total - how many items. */
void IndexTreeWriter::writeStringIndex(ktree::tree_node<inxtree_chrindex>::treeNodePtr parent,
                                        int len_inx, int* total, off_t *start, FILE* file)
{
	static u32 index[STRINX_LEN_MAX] = {0};

	for (int i=0; i<parent->size(); i++) {
		struct inxtree_chrindex& charIndex = (*parent)[i]->value();
		u32 wchr = inxtree_read_u32(charIndex.wchr);
		if (len_inx < STRINX_LEN_MAX) {
			index[len_inx] = wchr;
		} else {
			printf(
			"WARRING: length of string index greatter then STRINX_LEN_MAX,please check function is_in_stringindex\n");
            index[STRINX_LEN_MAX-1] = wchr;
		}

		if (inxtree_read_u32(charIndex.location) != INXTREE_INVALID_ADDR) {
			int nbytes_strinx = sizeof( struct inxtree_strindex)-1;
            size_t  nbytes_str=0;
            index[len_inx+1] = L'\0';
            char* mbindex = IndexTreeHelper::ucs4StrToUTF8Str(index, &nbytes_str);

            if (mbindex != NULL) {
                if (nbytes_str == (size_t) -1) {
                    nbytes_str = strlen(mbindex);
                    printf("WARRING: part of index was converted to utf-8, length(%lu)\n", nbytes_str);
                }
                //printf("%s-->%lu\n", mbindex, nbytes_str);
                nbytes_strinx += nbytes_str;
			    struct inxtree_strindex *strinx = ( struct inxtree_strindex *)malloc(nbytes_strinx);
			    memcpy(strinx->keystr, mbindex, nbytes_str);
			    memcpy(strinx->location, charIndex.location, sizeof(strinx->location));
			    strinx->len_str[0] = nbytes_str;

			    off_t offset = IndexTreeHelper::checkBlockBound(ftello(file), nbytes_strinx);
			    fseek(file, offset, SEEK_CUR);
                if ((*total) == 0) {
                    *start = ftello(file);
                }
			    fwrite(strinx, nbytes_strinx, 1, file);
			    free(strinx);
                free(mbindex);
			    (*total) += 1;
            } else {
                printf("ERROR: index did't be converted to utf-8:(%x) \n", wchr);
            }
		}
		writeStringIndex((*parent)[i], len_inx+1, total, start, file);
	}
}

/*
 * After trimming index tree, only char index left, write it to file.
 */
void IndexTreeWriter::writeCharIndex(ktree::tree_node<inxtree_chrindex>::treeNodePtr parent,
                                     FILE* cinxfile)
{
	/*
	 * Deal with three situation (see figure 2):
	 *     1) parent is a char index and has children().
	 *     2) parent[i] is a node with string index.
	 *     3) parent[i] is only a non-leaf char index node.
	 */
	/* Reserve room for children, children should be save together at parent's location.*/
	m_totalChrindex += parent->size(); /* as a global variable, always point to the last file position */
	for (int i = 0; i < parent->size(); i++) {
		struct inxtree_chrindex& i_cinx = (*parent)[i]->value();
		int loc = inxtree_read_u32(i_cinx.location);
		int clen = (*parent)[i]->size();

		if (loc == INXTREE_INVALID_ADDR) { /* Situation 3 */
			inxtree_write_u16(i_cinx.len_content, clen);
			/* Reserve room for parent[i]'s children.
			   Write sequentialy from the end of file.
			   So, m_totalChrindex must be a global variable. */
			inxtree_write_u32(i_cinx.location, m_totalChrindex*sizeof(struct inxtree_chrindex));
		} else if ((*parent)[i]->size() > 0) { /* Situation 1 */
			struct inxtree_chrindex inx;

			inxtree_write_u32(inx.location, loc);
			inxtree_write_u32(inx.wchr, 0);
			inxtree_write_u16(inx.len_content, 0);
			(*parent)[i]->insert(inx, 0); /* Add a '0' index specifing the location in data area */

			inxtree_write_u16(i_cinx.len_content, clen+1);
			inxtree_write_u32(i_cinx.location, m_totalChrindex*sizeof(struct inxtree_chrindex));
		}

	    // Children's room has been reserved in parent's location.
	    off_t offset = inxtree_read_u32(parent->value().location) + i * sizeof(struct inxtree_chrindex);
	    fseek(cinxfile, (INDEX_BLOCK_NR-1)*INXTREE_BLOCK+offset, SEEK_SET);
	    fwrite(&i_cinx, sizeof(struct inxtree_chrindex), 1, cinxfile);
	    writeCharIndex((*parent)[i], cinxfile);
	}
}

struct inxtree_dataitem
IndexTreeWriter::dataitem(address_t loc)
{
	if (loc != INXTREE_INVALID_ADDR) {
        if (loc < m_inxDataLen) {
            map<int, struct inxtree_dataitem>::iterator iter = m_updateCache.find(loc);
            if(iter != m_updateCache.end()) {
                return iter->second;
            }

            off_t off = (m_dataLoc-1)*INXTREE_BLOCK + loc;
            return IndexTree::dataitem(m_inxFile, off);
        }
        return IndexTree::dataitem(m_dataTmpFile, loc - m_inxDataLen);
    }

    struct inxtree_dataitem d;
	memset(&d, 0, sizeof(struct inxtree_dataitem));
    return d;
}

bool IndexTreeWriter::update(const string& key, u8 *ptr, int len)
{
    indextree::MutexLock lock(m_cs);

    struct LookupStat lookupStat;
    lookupStat.advance = key;
    if (IndexTree::lookup((char*)key.c_str(), m_indexTree->root(), lookupStat)) {
        for (int i=0; i<lookupStat.locs.size(); i++) {
            address_t loc = lookupStat.locs[i];
            if (loc != INXTREE_INVALID_ADDR) {
                if (loc < m_inxDataLen) {
                    // Is a disk file, cache it.
                    struct inxtree_dataitem d;
                    memset(&d, 0, sizeof(struct inxtree_dataitem));  // init: d.ptr is not NULL

                    d.len_data = len;
                    if (m_dataItemSize > 0)
                        memcpy(d.buf, ptr, len);
                    else {
                        // TODO:
                    }
                    m_updateCache[loc] = d;
                    if (m_updateCache.size() > MAX_CACHE_TH)
                        sync();
                } else {
                    loc -= m_inxDataLen;
                    fseek(m_dataTmpFile, loc, SEEK_SET);printf("update tmp  %d\n", loc);
                    fwrite(ptr, len, 1, m_dataTmpFile);
                }
            }
        }
        return true;
    }
    return false;
}

void IndexTreeWriter::sync()
{
    map<int, struct inxtree_dataitem >::iterator iter;
    for (iter = m_updateCache.begin(); iter != m_updateCache.end(); iter++) {
        off_t loc = iter->first;
        struct inxtree_dataitem& d = iter->second;

        off_t off = (m_dataLoc-1)*INXTREE_BLOCK + loc;
        fseek(m_inxFile, off, SEEK_SET);
        if (m_dataItemSize > 0)
            fwrite(d.buf, d.len_data, 1, m_inxFile);
        else
            ; //TODO
    }
    m_updateCache.clear();

    fsync(fileno(m_inxFile));
}

int IndexTreeWriter::readData(u8 **ptr)
{
    indextree::MutexLock lock(m_cs);

    sync();

    int size;

    fseek(m_dataTmpFile, 0, SEEK_END);
    size = m_inxDataLen + ftello(m_dataTmpFile);

    // Read inxFile
    *ptr = (u8 *)malloc(size);
    if (*ptr != NULL) {

        {
            indextree::ReadFile read;
            off_t off = (m_dataLoc-1)*INXTREE_BLOCK;
            fseek(m_inxFile, off, SEEK_SET);
            read(m_inxFile, *ptr, m_inxDataLen);
        }

        {
            indextree::ReadFile read;
            fseek(m_dataTmpFile, 0, SEEK_SET);
            read(m_dataTmpFile, *ptr + m_inxDataLen, size - m_inxDataLen);
        }

        return size;
    }

    return 0;
}

void IndexTreeWriter::writeData(u8 *ptr, int size)
{
    indextree::MutexLock lock(m_cs);

    off_t off = (m_dataLoc-1)*INXTREE_BLOCK;
    fseek(m_inxFile, off, SEEK_SET);
    if (size <= m_inxDataLen) {
        fwrite(ptr, size, 1, m_inxFile);
    } else {
        fwrite(ptr, m_inxDataLen, 1, m_inxFile);

        fseek(m_dataTmpFile, 0, SEEK_SET);
        fwrite(ptr + m_inxDataLen, size - m_inxDataLen, 1, m_dataTmpFile);
    }

    //fflush(m_inxFile);
    //fflush(m_dataTmpFile);
    //fsync(fileno(m_inxFile));
    //fsync(fileno(m_dataTmpFile));
}
