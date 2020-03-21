/*
 * ====================================================================
 * Copyright (c) 2002-2018 The RapidSVN Group.  All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program (in the file LGPL.txt).
 * If not, see <http://www.gnu.org/licenses/>.
 *
 * This software consists of voluntary contributions made by many
 * individuals.  For exact contribution history, see the revision
 * history and logs, available at http://rapidsvn.tigris.org/.
 * ====================================================================
 */
// subversion api
#include "svn_client.h"
#include "svn_path.h"
#include "svn_sorts.h"
#include "svn_version.h"
//#include "svn_utf.h"

// svncpp
#include "svncpp/client.hpp"
#include "svncpp/dirent.hpp"
#include "svncpp/exception.hpp"

#include "m_is_empty.hpp"


#if SVN_VER_MAJOR == 1 && SVN_VER_MINOR < 8
static int
compare_items_as_paths(const svn_sort__item_t *a, const svn_sort__item_t *b)
{
  return svn_path_compare_paths((const char *)a->key, (const char *)b->key);
}

namespace svn
{
  static svn_error_t*
  listEntriesFunc(void *baton, const char *path,
                  const svn_dirent_t *dirent, const svn_lock_t *lock,
                  const char *abs_path, apr_pool_t *pool)
  {
    if (!isEmpty(path))
    {
      DirEntries * entries = static_cast<DirEntries *>(baton);
      entries->push_back(
        DirEntry(path, const_cast<svn_dirent_t *>(dirent), lock));
    }
    return 0;
  }

  DirEntries
  Client::list(const char * pathOrUrl,
               svn_opt_revision_t * revision,
               bool recurse)
  {
    Pool pool;

    DirEntries entries;
    svn_error_t * error =
      svn_client_list2(pathOrUrl,
                       revision,
                       revision,
                       recurse ? svn_depth_infinity : svn_depth_immediates,
                       SVN_DIRENT_ALL,
                       true,
                       listEntriesFunc,
                       &entries,
                       *m_context,
                       pool);

    if (error != 0)
      throw ClientException(error);

    return entries;
  }
}

#else

#include <algorithm>

static svn_error_t* store_entry(
  void *baton,
  const char *path,
  const svn_dirent_t *dirent,
  const svn_lock_t *,
  const char *abs_path,
  const char *,
  const char *,
  apr_pool_t *scratch_pool)
{
  svn::DirEntries *entries = reinterpret_cast<svn::DirEntries*>(baton);
  if (path[0] == '\0') {
    if (dirent->kind == svn_node_file) {
      // for compatibility with svn_client_ls behaviour, listing a file
      // stores that file name
      entries->push_back(svn::DirEntry(svn_path_basename(abs_path, scratch_pool), dirent));
    }
  } else {
    entries->push_back(svn::DirEntry(path, dirent));
  }
  return SVN_NO_ERROR;
}

static bool sort_by_path(svn::DirEntry const& a, svn::DirEntry const& b)
{
  return svn_path_compare_paths(a.name(), b.name()) < 0;
}

namespace svn
{
  DirEntries
  Client::list(const char * pathOrUrl,
               svn_opt_revision_t * revision,
               bool recurse)
  {
    Pool pool;
    DirEntries entries;

    svn_error_t * error =
      svn_client_list3(pathOrUrl,
                       revision,
                       revision,
                       SVN_DEPTH_INFINITY_OR_IMMEDIATES(recurse),
                       SVN_DIRENT_ALL,
                       FALSE, // fetch locks
                       FALSE, // include externals
                       &store_entry,
                       &entries,
                       *m_context,
                       pool);

    if (error != SVN_NO_ERROR)
      throw ClientException(error);

    std::sort(entries.begin(), entries.end(), &sort_by_path);

    return entries;
  }
}

#endif

/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
