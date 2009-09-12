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

#ifndef SUNPINYIN_KEYSDEFINE_H
#define SUNPINYIN_KEYSDEFINE_H

#include "portability.h"

#define IM_SHIFT_MASK        (1 << 0)
#define IM_CTRL_MASK         (1 << 2)
#define IM_ALT_MASK          (1 << 3)
#define IM_RELEASE_MASK      (1 << 30)

#define IM_VK_SPACE          ' '
#define IM_VK_MINUS          '-'
#define IM_VK_EQUALS         '='
#define IM_VK_COMMA          ','
#define IM_VK_PERIOD         '.'
#define IM_VK_OPEN_BRACKET   '['
#define IM_VK_CLOSE_BRACKET  ']'
#define IM_VK_BACK_QUOTE     '`'

#define IM_VK_HOME           0xff50
#define IM_VK_LEFT           0xff51
#define IM_VK_UP             0xff52
#define IM_VK_RIGHT          0xff53
#define IM_VK_DOWN           0xff54
#define IM_VK_PAGE_UP        0xff55
#define IM_VK_PAGE_DOWN      0xff56
#define IM_VK_END            0xff57

#define IM_VK_DELETE         0xffff
#define IM_VK_BACK_SPACE     0xff08

#define IM_VK_ENTER          0xff0d
#define IM_VK_ESCAPE         0xff1b

#define IM_VK_SHIFT          0xffe1
#define IM_VK_CONTROL        0xffe3
#define IM_VK_ALT            0xffe9

#endif
