/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 * Copyright (c) 2007 Sun Microsystems, Inc. All Rights Reserved.
 *
 * The contents of this file are subject to the terms of either the GNU Lesser
 * General Public License Version 2.1 only ("LGPL") or the Common Development and
 * Distribution License ("CDDL")(collectively, the "License"). You may not use this
 * file except in compliance with the License. You can obtain a copy of the CDDL at
 * http://www.opensource.org/licenses/cddl1.php and a copy of the LGPLv2.1 at
 * http://www.opensource.org/licenses/lgpl-license.php. See the License for the
 * specific language governing permissions and limitations under the License. When
 * distributing the software, include this License Header Notice in each file and
 * include the full text of the License in the License file as well as the
 * following notice:
 *
 * NOTICE PURSUANT TO SECTION 9 OF THE COMMON DEVELOPMENT AND DISTRIBUTION LICENSE
 * (CDDL)
 * For Covered Software in this distribution, this License shall be governed by the
 * laws of the State of California (excluding conflict-of-law provisions).
 * Any litigation relating to this License shall be subject to the jurisdiction of
 * the Federal Courts of the Northern District of California and the state courts
 * of the State of California, with venue lying in Santa Clara County, California.
 *
 * Contributor(s):
 *
 * If you wish your version of this file to be governed by only the CDDL or only
 * the LGPL Version 2.1, indicate your decision by adding "[Contributor]" elects to
 * include this software in this distribution under the [CDDL or LGPL Version 2.1]
 * license." If you don't indicate a single choice of license, a recipient has the
 * option to distribute your version of this file under either the CDDL or the LGPL
 * Version 2.1, or to extend the choice of license to its licensees as provided
 * above. However, if you add LGPL Version 2.1 code and therefore, elected the LGPL
 * Version 2 license, then the option applies only if the new code is made subject
 * to such option by the copyright holder.
 */

#ifdef HAVE_CONFIG_H
#if defined(_WIN32) && defined(_MSC_VER)
#include <config-win32-msvc.h>
#else
#include <config.h>
#endif
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <cassert>
#ifndef _WIN32
#include <unistd.h>
#include <arpa/inet.h>
#else
#include <windows.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm>
#include "ic_history.h"

const uint32_t CICHistory::DCWID = (uint32_t)-1;

CICHistory::~CICHistory()
{
}

const size_t CBigramHistory::contxt_memory_size = 8192;
const double CBigramHistory::focus_memory_ratio = 0.05;

//FIXME: CBigramHistory need to be thread safe
CBigramHistory::CBigramHistory() : m_memory(), m_unifreq(), m_bifreq()
{
    initStopWords();
}

CBigramHistory::~CBigramHistory()
{
}

bool
CBigramHistory::memorize(uint32_t* its_wid, uint32_t* ite_wid)
{
    TBigram bigram(DCWID, DCWID);

    // First, we insert a DC word id before the context history
    // to separated from previous stream.
    if (m_memory.size() == contxt_memory_size) {
        TBigram hb;
        hb.first = m_memory.front();
        m_memory.pop_front();
        hb.second = m_memory.front();

        decUniFreq(hb.first);
        decBiFreq(hb);
    }
    m_memory.push_back(DCWID);

    //Now trying to memorize new stream and forget oldest
    for (; its_wid != ite_wid; ++its_wid) {
        if (m_memory.size() == contxt_memory_size) {
            TBigram hb;
            hb.first = m_memory.front();
            m_memory.pop_front();
            hb.second = m_memory.front();

            decUniFreq(hb.first);
            decBiFreq(hb);
        }
        bigram.first = bigram.second;
        bigram.second = *its_wid;
        m_memory.push_back(*its_wid);
        incUniFreq(bigram.second);
        incBiFreq(bigram);
    }
    return true;
}

void
CBigramHistory::clear()
{
    m_memory.clear();
    m_unifreq.clear();
    m_bifreq.clear();
}

double
CBigramHistory::pr(uint32_t* its_wid, uint32_t* ite_wid)
{
    TBigram bigram(DCWID, DCWID);
    if (its_wid != ite_wid) {
        --ite_wid;
        bigram.second = *ite_wid;
        if (its_wid != ite_wid)
            bigram.first = *(ite_wid - 1);
    }
    return pr(bigram);
}

double
CBigramHistory::pr(uint32_t* its_wid, uint32_t* ite_wid, uint32_t wid)
{
    TBigram bigram(DCWID, DCWID);
    if (its_wid != ite_wid)
        bigram.first = *(ite_wid - 1);
    bigram.second = wid;
    return pr(bigram);
}

// htonl could be a macro, so wrap it in a func.
inline uint32_t
swap32(uint32_t x)
{
#ifndef _WIN32
    return htonl(x);
#else
    return ((x << 24) | ((x & 0x0000ff00) << 8) | ((x & 0x00ff0000) >> 8) | (x >> 24));
#endif
}

bool
CBigramHistory::bufferize(void** buf_ptr, size_t* sz)
{
    *buf_ptr = NULL;
    *sz = 0;
    try {
        *sz = sizeof(uint32_t) * m_memory.size();
        if (*sz > 0) {
            *buf_ptr = malloc(*sz); // malloc for C compatible
#ifdef WORDS_BIGENDIAN
            std::copy(m_memory.begin(), m_memory.end(), (uint32_t*)*buf_ptr);
#else
            std::transform(m_memory.begin(),
                           m_memory.end(), (uint32_t*)*buf_ptr, swap32);
#endif
        }
        return true;
    } catch (...) {
        if (*buf_ptr)
            free(*buf_ptr);
        *buf_ptr = NULL;
        *sz = 0;
    }
    return false;
}

bool
CBigramHistory::loadFromFile(const char *fname)
{
    m_history_path = fname;
    bool suc = false;

#ifndef _WIN32
    int fd = open(fname, O_CREAT, 0600);
    if (fd == -1) {
        suc = loadFromBuffer(NULL, 0);
        return suc;
    }

    struct stat info;
    fstat(fd, &info);
    void* buf = malloc(info.st_size);

    if (buf) {
        read(fd, buf, info.st_size);
        suc = loadFromBuffer(buf, info.st_size);
        free(buf);
    }
    close(fd);
#else // _WIN32
    HANDLE fd = CreateFile(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fd != INVALID_HANDLE_VALUE) {
            LARGE_INTEGER li;
            li.HighPart = 0;

            SetFilePointer(fd, 0, NULL, FILE_END);
            if((li.LowPart = SetFilePointer(fd, 0, &li.HighPart, FILE_CURRENT)) != -1 && li.HighPart == 0) {
                void* buf = malloc((size_t)li.QuadPart);
                if (buf) {
                        DWORD nRead = 0;
                        SetFilePointer(fd, 0, NULL, FILE_BEGIN);
                        ReadFile(fd, buf, (DWORD)li.QuadPart, &nRead, NULL);
                        if (nRead == (DWORD)li.QuadPart)
                            suc = loadFromBuffer(buf, (size_t)li.QuadPart);
                        free(buf);
                }
            }
            CloseHandle(fd);
    }
    else
    {
        suc = loadFromBuffer(NULL, 0);
    }
#endif
    return suc;
}

bool
CBigramHistory::saveToFile(const char *fname)
{
    if (!fname)
        fname = m_history_path.c_str();

    bool suc = false;
    size_t sz = 0;
    void* buf = NULL;
#ifndef _WIN32
    if (bufferize(&buf, &sz) && buf) {
        FILE* fp = fopen(fname, "wb");
        if (fp) {
            suc = (fwrite(buf, 1, sz, fp) == sz);
            fclose(fp);
        }
        free(buf);
    }
#else
    if (bufferize(&buf, &sz) && buf) {
            HANDLE fd = CreateFile(fname,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (fd != INVALID_HANDLE_VALUE) {
                    DWORD nWrote = (DWORD)sz;
                    SetFilePointer(fd, 0, NULL, FILE_BEGIN);
                    WriteFile(fd, buf, nWrote, &nWrote, NULL);
                    suc = (nWrote == (DWORD)sz);
                    CloseHandle(fd);
            }
    }
#endif
    return suc;
}

bool
CBigramHistory::loadFromBuffer(void* buf_ptr, size_t sz)
{
    clear();

    sz /= sizeof(uint32_t);
    uint32_t *pw = (uint32_t*)buf_ptr;

    if (pw && sz > 0) {
#ifndef WORDS_BIGENDIAN
        std::transform(pw, pw + sz, pw, swap32);
#endif
        TBigram bigram(DCWID, DCWID);
        for (size_t i = 0; i < sz; ++i) {
            bigram.first = bigram.second;
            bigram.second = *pw++;
            m_memory.push_back(bigram.second);
            incUniFreq(bigram.second);
            incBiFreq(bigram);
        }
    }
    return true;
}

double
CBigramHistory::pr(TBigram& bigram)
{
    int uf0 = uniFreq(bigram.first);
    int bf = biFreq(bigram);
    int uf1 = uniFreq(bigram.second);

    double pr = 0.0;
    pr += 0.68 * double(bf) / double(uf0 + 0.5);
    pr += 0.32 * double(uf1) /
          double(m_memory.size() + (contxt_memory_size - m_memory.size()) / 10);

#ifdef DEBUG
    if (pr != 0)
        fprintf(stderr, "uf0:%d bf:%d uf1:%d pr(%d|%d):%lf\n", uf0, bf, uf1,
                bigram.second, bigram.first, pr);
#endif

    return pr;
}

int
CBigramHistory::uniFreq(TUnigram& ug)
{
    int freq = 0;
    if (m_stopWords.find(ug) == m_stopWords.end()) {
        TUnigramPool::iterator it = m_unifreq.find(ug);
        if (it != m_unifreq.end()) {
            freq = it->second;
            TContextMemory::reverse_iterator rit = m_memory.rbegin();
            for (int i =
                     0;
                 rit != m_memory.rend() && i < contxt_memory_size *
                 focus_memory_ratio;
                 i++) {
                if (*rit == ug)
                    freq += 1.0 / focus_memory_ratio;
                rit++;
            }
        }
    }
    //if (freq != 0) printf("uniFreq[%d]-->%d\n", ug, freq);
    return freq / 2;
}

int
CBigramHistory::biFreq(TBigram& bg)
{
    int freq = 0;
    //std::set<unsigned>::const_iterator ite = m_stopWords.end();
    if (m_stopWords.find(bg.first) == m_stopWords.end()
        && m_stopWords.find(bg.second) == m_stopWords.end()) {
        TBigramPool::const_iterator it = m_bifreq.find(bg);
        if (it != m_bifreq.end()) {
            freq = it->second;
            TContextMemory::reverse_iterator re = m_memory.rbegin();
            TContextMemory::reverse_iterator rs = re + 1;
            for (int i = 0;
                 rs != m_memory.rend() && i < contxt_memory_size *
                 focus_memory_ratio;
                 i++) {
                if (*rs == bg.first && *re == bg.second)
                    freq += 1.0 / focus_memory_ratio;
                ++rs; ++re;
            }
        }
    }

    //if (freq != 0) printf("biFreq[%d,%d]-->%d\n", bg.first, bg.second, freq);
    return freq;
}

void
CBigramHistory::decUniFreq(TUnigram& ug)
{
    TUnigramPool::iterator it = m_unifreq.find(ug);
    if (it != m_unifreq.end()) {
        if (it->second > 1)
            --(it->second);
        else
            m_unifreq.erase(it);
    }
}

bool
CBigramHistory::seenBefore(uint32_t wid)
{
    return(wid != DCWID && m_stopWords.find(wid) == m_stopWords.end() &&
           m_unifreq.find(wid) != m_unifreq.end());
}

void
CBigramHistory::decBiFreq(TBigram& bg)
{
    TBigramPool::iterator it = m_bifreq.find(bg);
    if (it != m_bifreq.end()) {
        if (it->second > 1)
            --(it->second);
        else
            m_bifreq.erase(it);
    }
}

void
CBigramHistory::incUniFreq(TUnigram& ug)
{
    ++m_unifreq[ug];
    //printf("Remembering uniFreq[%d]-->%d\n", ug, m_unifreq[ug]);
}

void
CBigramHistory::incBiFreq(TBigram& bg)
{
    ++m_bifreq[bg];
    //printf("Remembering biFreq[%d,%d]-->%d\n", bg.first, bg.second, m_bifreq[bg]);
}

// so far, it's very expensive to erase a word from bigram pairs, need to design
// a better data structure for this.
//
// And Even though, we may also need to remove the individual characters in this
// word (identified by wid), which is current infeasible,
//
// Here are what we need to do:
//   1. get the wstring by word id from userdict
//   2. iterate the character in this wstring
//   3. get the word id from each character from system lexicon (not supported yet)
//   4. remove the unigrams and bigrams of each character, and the entire word
//
void
CBigramHistory::forget(uint32_t wid)
{
    TUnigramPool::iterator uni_it = m_unifreq.find(wid);
    if (uni_it != m_unifreq.end())
        m_unifreq.erase(uni_it);

    TBigramPool::iterator it = m_bifreq.begin();
    TBigramPool::iterator ite = m_bifreq.end();

    while (it != ite) {
        TBigram bigram = it->first;

        if (bigram.first == wid || bigram.second == wid)
            m_bifreq.erase(it++);
        else
            ++it;
    }
}

void
CBigramHistory::forget(uint32_t *its_wid, uint32_t *ite_wid)
{
    for (; its_wid < ite_wid; ++its_wid) {
        TBigram bigram(*its_wid, DCWID);

        if (its_wid + 1 != ite_wid)
            bigram.second = *(its_wid + 1);

        TBigramPool::iterator it = m_bifreq.find(bigram);
        if (it != m_bifreq.end())
            m_bifreq.erase(it);
    }
}

void
CBigramHistory::addStopWords(const std::set<uint32_t>& stopWords)
{
    m_stopWords.insert(stopWords.begin(), stopWords.end());
}

void
CBigramHistory::initStopWords()
{
    m_stopWords.clear();

    m_stopWords.insert(0);     //unknown world
    m_stopWords.insert(DCWID); //separator word id used by history memory internally
}

// -*- indent-tabs-mode: nil -*- vim:et:ts=4
