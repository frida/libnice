/*
 * This file is part of the Nice GLib ICE library.
 *
 * (C) 2006, 2007 Collabora Ltd.
 *  Contact: Dafydd Harries
 * (C) 2006, 2007 Nokia Corporation. All rights reserved.
 *  Contact: Kai Vehmanen
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Nice GLib ICE library.
 *
 * The Initial Developers of the Original Code are Collabora Ltd and Nokia
 * Corporation. All Rights Reserved.
 *
 * Contributors:
 *   Dafydd Harries, Collabora Ltd.
 *
 * Alternatively, the contents of this file may be used under the terms of the
 * the GNU Lesser General Public License Version 2.1 (the "LGPL"), in which
 * case the provisions of LGPL are applicable instead of those above. If you
 * wish to allow use of your version of this file only under the terms of the
 * LGPL and not to allow others to use your version of this file under the
 * MPL, indicate your decision by deleting the provisions above and replace
 * them with the notice and other provisions required by the LGPL. If you do
 * not delete the provisions above, a recipient may use your version of this
 * file under either the MPL or the LGPL.
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "agent.h"
#include "agent-priv.h"
#include "interfaces.h"
#include "candidate.h"


static void
test_priority (void)
{
  NiceCandidate *candidate;
  GSList *local_addresses;
  NiceAddress *addr;
  guint16 ip_local_preference = 0;

  candidate = nice_candidate_new (NICE_CANDIDATE_TYPE_HOST);
  nice_address_set_from_string (&candidate->addr, "127.0.0.1");
  nice_address_set_from_string (&candidate->base_addr, "127.0.0.1");

  addr = nice_address_dup (&candidate->addr);
  local_addresses = g_slist_append (NULL, addr);

  /* test 0 */
  g_assert_cmpuint (ip_local_preference, <, NICE_CANDIDATE_MAX_LOCAL_ADDRESSES);

  /* test 1 */
  g_assert_cmpuint (nice_candidate_jingle_priority (candidate), ==, 1000);
  /* Host UDP */
  candidate->transport = NICE_CANDIDATE_TRANSPORT_UDP;
  candidate->component_id = 1;
  g_assert_cmpuint (nice_candidate_ice_priority (candidate, FALSE, FALSE, local_addresses), ==, 0x782000FF + 0x100 * ip_local_preference );
  /* Host UDP reliable */
  g_assert_cmpuint (nice_candidate_ice_priority (candidate, TRUE, FALSE, local_addresses), ==, 0x3C2000FF + 0x100 * ip_local_preference );
  /* Host tcp-active unreliable */
  candidate->transport = NICE_CANDIDATE_TRANSPORT_TCP_ACTIVE;
  g_assert_cmpuint (nice_candidate_ice_priority (candidate, FALSE, FALSE, local_addresses) & 0xFFE000FF, ==, 0x3C8000FF);
  /* Host tcp-active reliable */
  candidate->transport = NICE_CANDIDATE_TRANSPORT_TCP_ACTIVE;
  /* Host tcp-active reliable */
  g_assert_cmpuint (nice_candidate_ice_priority (candidate, TRUE, FALSE, local_addresses) & 0xFFE000FF, ==, 0x788000FF);
  /* srv-reflexive tcp-active reliable */
  candidate->type = NICE_CANDIDATE_TYPE_SERVER_REFLEXIVE;
  candidate->transport = NICE_CANDIDATE_TRANSPORT_TCP_ACTIVE;
  g_assert_cmpuint (nice_candidate_ice_priority (candidate, TRUE, FALSE, local_addresses) & 0xFFE000FF, ==, 0x648000FF);
  /* nat-assisted srv-reflexive tcp-active reliable */
  g_assert_cmpuint (nice_candidate_ice_priority (candidate, TRUE, TRUE, local_addresses) & 0xFFE000FF, ==, 0x698000FF);
  nice_candidate_free (candidate);

  /* test 2 */
  /* 2^32*MIN(O,A) + 2*MAX(O,A) + (O>A?1:0)
     = 2^32*1 + 2*5000 + 0
     = 4294977296 */
  g_assert_cmpuint (nice_candidate_pair_priority (1,5000), ==, 4294977296LL);

  /* 2^32*1 + 2*5000 + 1 = 4294977297 */
  g_assert_cmpuint (nice_candidate_pair_priority (5000, 1), ==, 4294977297LL);

  g_slist_free_full (local_addresses, (GDestroyNotify)&nice_address_free);
}

int
main (void)
{
#ifdef G_OS_WIN32
  WSADATA w;
#endif

#ifdef G_OS_WIN32
  WSAStartup(0x0202, &w);
#endif
  test_priority ();

#ifdef G_OS_WIN32
  WSACleanup();
#endif
  return 0;
}
