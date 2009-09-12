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

#ifndef SUNPY_IMI_SESSION_VIEW_H
#define SUNPY_IMI_SESSION_VIEW_H

#include "portability.h"

#include "imi_context.h"
#include "imi_winHandler.h"
#include "imi_uiobjects.h"
#include "imi_keys.h"
#include "utils.h"
#include <set>

struct CKeyEvent
{
    unsigned code;
    unsigned value; 
    unsigned modifiers;
    
    CKeyEvent (unsigned kc, unsigned kv = 0, unsigned m = 0)
        : code(kc), value(kv), modifiers(m)
    {
        // clear other mask bit we do not care
        modifiers &= (IM_SHIFT_MASK | IM_CTRL_MASK | IM_ALT_MASK | IM_RELEASE_MASK);
    }
    
    bool operator < (const CKeyEvent& b) const
    {
        return ((code < b.code) ||
                ((code == b.code) &&
                 ((value < b.value) ||
                  ((value == b.value) &&
                   (modifiers < b.modifiers)))));
    }
    
    bool operator == (const CKeyEvent& rhs) const
    {
        return (code == rhs.code &&
                modifiers == rhs.modifiers);
    }
    
    bool operator != (const CKeyEvent& rhs) const
    {
        return (code != rhs.code ||
                modifiers != rhs.modifiers);
    }
    
    void reset()
    {
        code = (unsigned)~0;
        value = (unsigned)~0;
        modifiers = 0;
    }
};

class CHotkeyProfile : private CNonCopyable
{
public:
    CHotkeyProfile();
    
    void clear () 
    {
        m_pageUpKeys.clear();
        m_pageDownKeys.clear();
    }

    void addPageUpKey (const CKeyEvent& key)
    {
        m_pageUpKeys.insert (key);
    }

    bool isPageUpKey  (const CKeyEvent& key) const
    {
        return (m_pageUpKeys.find (key) != m_pageUpKeys.end());
    }

    void addPageDownKey (const CKeyEvent& key)
    {
        m_pageDownKeys.insert (key);
    }

    bool isPageDownKey  (const CKeyEvent& key) const
    {
        return (m_pageDownKeys.find (key) != m_pageDownKeys.end());
    }

    void setModeSwitchKey (const CKeyEvent& key)
    {
        m_modeSwitchKey = key;
    }
    
    bool isModeSwitchKey (const CKeyEvent& key) const
    {
        return m_modeSwitchKey == key;
    }
    
    void setPunctSwitchKey (const CKeyEvent& key)
    {
        m_punctSwitchKey = key;
    }
    
    bool isPunctSwitchKey (const CKeyEvent& key) const
    {
        return m_punctSwitchKey == key;
    }
    
    void setSymbolSwitchKey (const CKeyEvent& key)
    {
        m_symbolSwitchKey = key;
    }
    
    bool isSymbolSwitchKey (const CKeyEvent& key) const
    {
        return m_symbolSwitchKey == key;
    }
    
protected:
    std::set<CKeyEvent> m_pageUpKeys;
    std::set<CKeyEvent> m_pageDownKeys;
    CKeyEvent           m_modeSwitchKey;
    CKeyEvent           m_punctSwitchKey;
    CKeyEvent           m_symbolSwitchKey;
};

class CIMIView {
public:
    enum {
        KEYEVENT_USED  = (1),
        PREEDIT_MASK   = (1<<2),
        CANDIDATE_MASK = (1<<3)
    };

public:
    CIMIView();
    virtual ~CIMIView() {}

    void attachIC(CIMIContext* pIC) {m_pIC = pIC;}
    CIMIContext* getIC(void) {return m_pIC;}

    void setPySegmentor (IPySegmentor *p) {m_pPySegmentor = p;}
    IPySegmentor* getPySegmentor () {return m_pPySegmentor;}

    void attachWinHandler(CIMIWinHandler* wh) {m_pWinHandler = wh;}
    CIMIWinHandler* getWinHandler(void) {return m_pWinHandler;}

    void setHotkeyProfile (CHotkeyProfile *prof) { m_pHotkeyProfile = prof;}
    void setCandiWindowSize (unsigned size) {m_candiWindowSize = size<10? size: 10;}

    virtual unsigned clearIC(void) {m_pIC->clear(); return 0;}
    virtual bool onKeyEvent(const CKeyEvent& key) {return false;}

    virtual void setStatusAttrValue(int key, int value);
    virtual int  getStatusAttrValue(int key);
    virtual void updateWindows(unsigned mask) = 0;

    virtual void getPreeditString(IPreeditString& ps) = 0;
    virtual void getCandidateList(ICandidateList& cl, int start, int size) = 0;

    virtual int  onCandidatePageRequest(int pgno, bool relative) = 0; //pgno == -1, relative == false means last page
    virtual int  onCandidateSelectRequest(int index) = 0;

protected:
    CIMIContext        *m_pIC;
    CIMIWinHandler     *m_pWinHandler;
    IPySegmentor       *m_pPySegmentor;
    CHotkeyProfile     *m_pHotkeyProfile;

    unsigned            m_candiWindowSize;

    bool                m_bCN;
    bool                m_bFullPunct;
    bool                m_bFullSymbol;
};

#endif
