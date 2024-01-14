/* strl_utils  (From BSD and libbsd)
 *
 *  $OpenBSD: strlcpy.c,v 1.8 2003/06/17 21:56:24 millert Exp $
 *  $OpenBSD: strlcat.c,v 1.11 2003/06/17 21:56:24 millert Exp $
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef tps_strl_utils_h
#define tps_strl_utils_h

#include <stdlib.h> /* for size_t */

size_t tps_strlcpy(char *dst, const char *src, size_t siz);
size_t tps_strlcat(char *dst, const char *src, size_t siz);

#endif
