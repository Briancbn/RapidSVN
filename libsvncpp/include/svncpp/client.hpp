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

#ifndef _SVNCPP_CLIENT_H_
#define _SVNCPP_CLIENT_H_

// Ignore MSVC 7,8,9 compiler warnings
#if defined (_MSC_VER) && _MSC_VER > 1200 && _MSC_VER <= 1500
// C++ exception specification
#pragma warning (disable: 4290)
#endif


// stl
#include <assert.h>
#include "svncpp/vector_wrapper.hpp"
#include "svncpp/utility_wrapper.hpp"
#include "svncpp/map_wrapper.hpp"

// svncpp
#include "svncpp/context.hpp"
#include "svncpp/exception.hpp"
#include "svncpp/path.hpp"
#include "svncpp/entry.hpp"
#include "svncpp/revision.hpp"
#include "svncpp/log_entry.hpp"
#include "svncpp/annotate_line.hpp"


namespace svn
{
  // forward declarations
  class Context;
  class DirEntry;
  class Info;
  class Status;
  class Targets;

  typedef std::vector<AnnotateLine> AnnotatedFile;
  typedef std::vector<DirEntry> DirEntries;
  typedef std::vector<Info> InfoVector;
  typedef std::vector<LogEntry> LogEntries;
  typedef std::vector<Status> StatusEntries;


  // map of property names to values
  typedef std::map<std::string,std::string> PropertiesMap;
  // pair of path, PropertiesMap
  typedef std::pair<std::string, PropertiesMap> PathPropertiesMapEntry;
  // vector of path, Properties pairs
  typedef std::vector<PathPropertiesMapEntry> PathPropertiesMapList;

  /**
   * These flags can be passed to the status function to filter
   * the files
   *
   * @see status
   */
  struct StatusFilter
  {
  public:
    bool showUnversioned;
    bool showUnmodified;
    bool showModified;    ///< this includes @a showConflicted as well
    bool showConflicted;
    bool showIgnored;
    bool showExternals;

    StatusFilter()
      : showUnversioned(false), showUnmodified(false),
        showModified(false), showConflicted(false),
        showExternals(false)
    {
    }
  };


  struct CommitInfo
  {
    svn_revnum_t revision;

    /** server-side date of the commit. */
    std::string date;

    /** author of the commit. */
    std::string author;

    /** error message from post-commit hook */
    std::string postCommitErr;

    CommitInfo() :
      revision(SVN_INVALID_REVNUM)
    {
    }

    CommitInfo(const svn_commit_info_t *info)
    {
      assert(0 != info);

      revision = info->revision;
      date = info->date;
      author = info->author;
      if (0 != info->post_commit_err)
        postCommitErr = info->post_commit_err;
    }
  };


  /**
   * Subversion client API.
   */
  class Client
  {
  public:
    /**
     * Initializes the primary memory pool.
     */
    Client(Context * context = 0);

    virtual ~Client();

    /**
     * @since 0.14
     */
    const CommitInfo& commitInfo() const
    {
      return m_commitInfo;
    }

    /**
     * @return returns the Client context
     */
    const Context *
    getContext() const;

    /**
     * @return returns the Client context
     */
    Context *
    getContext();

    /**
     * sets the client context
     * you have to make sure the old context
     * is de-allocated
     *
     * @param context new context to use
     */
    void
    setContext(Context * context = NULL);

    /**
     * Enumerates all files/dirs at a given path.
     *
     * Throws an exception if an error occurs
     *
     * @param path Path to explore.
     * @param descend Recurse into subdirectories if existant.
     * @param get_all Return all entries, not just the interesting ones.
     * @param update Query the repository for updates.
     * @param no_ignore Disregard default and svn:ignore property ignores.
     * @param ignore_externals Disregard external files.
     * @exception ClientException If libsvn reports an error during the operation.
     * @return vector with Status entries.
     */
    StatusEntries
    status(const char * path,
           const bool descend = false,
           const bool get_all = true,
           const bool update = false,
           const bool no_ignore = false,
           const bool ignore_externals = false);

    /**
     * Enumerates all files/dirs matchin the parameter @a filter
     * at @a path and returns them in the vector @a statusEntries
     *
     * Throws an exception if an error occurs
     *
     * @since New in 0.9.7
     *
     * @param path Path to explore.
     * @param filter use a combination of the @a SHOW_* values to filter the
     *        output
     * @param descend Recurse into subdirectories if existant.
     * @param update Query the repository for updates.
     * @param entries vector with Status entries
     * @exception ClientException If libsvn reports an error during the operation.
     *
     * @return current revnum
     */
    svn_revnum_t
    status(const char * path,
           const StatusFilter & filter,
           const bool descend,
           const bool update,
           StatusEntries & entries);


    /**
     * Executes a revision checkout.
     * @param moduleName name of the module to checkout.
     * @param destPath destination directory for checkout.
     * @param revision the revision number to checkout. If the number is -1
     *                 then it will checkout the latest revision.
     * @param depth the checkout depth.
     * @param ignore_externals whether you want get external resources too.
     * @param peg_revision peg revision to checkout, by default current.
     * @exception ClientException If libsvn reports an error during the operation.
     */
    svn_revnum_t
    checkout(const char * moduleName,
             const Path & destPath,
             const Revision & revision,
             svn_depth_t depth,
             bool ignore_externals = false,
             const Revision & peg_revision = Revision::UNSPECIFIED);

    /**
     * relocate wc @a from to @a to
     * @exception ClientException If libsvn reports an error during the operation.
     */
    void
    relocate(const Path & path, const char *from_url,
             const char *to_url, bool recurse);

    /**
     * Sets a single file for deletion.
     * @exception ClientException If libsvn reports an error during the operation.
     */
    void
    remove(const Path & path, bool force);

    /**
     * Sets files for deletion.
     *
     * @param targets targets to delete
     * @param force force if files are locally modified
     * @exception ClientException If libsvn reports an error during the operation.
     */
    void
    remove(const Targets & targets,
           bool force);

    /**
     * Sets files to lock.
     *
     * @param targets targets to lock
     * @param force force setting/stealing lock
     * @param comment writing comment about lock setting is neccessary
     * @exception ClientException If libsvn reports an error during the operation.
     */
    void
    lock(const Targets & targets, bool force,
         const char * comment);

    /**
     * Sets files to unlock.
     *
     * @param targets targets to unlock
     * @param force force unlock even if lock belongs to another user
     * @exception ClientException If libsvn reports an error during the operation.
     */
    void
    unlock(const Targets & targets, bool force);

    /**
     * Reverts a couple of files to a pristiner state.
     * @exception ClientException If libsvn reports an error during the operation.
     */
    void
    revert(const Targets & targets, bool recurse);

    /**
     * Adds a file to the repository.
     * @exception ClientException If libsvn reports an error during the operation.
     */
    void
    add(const Path & path, bool recurse);

    /**
     * Updates the files or directories.
     * @param targets target files.
     * @param revision the revision number to checkout.
     *                 Revision::HEAD will checkout the
     *                 latest revision.
     * @param depth the update depth.
     *              The special value svn_depth_unknown fetches whatever was already there (the previous "sticky depth").
     * @param depth_is_sticky If depth != svn_depth_unknown, makes the depth sticky (it will be saved permanently).
     * @param ignore_externals don't affect external destinations.
     * @exception ClientException If libsvn reports an error during the operation.
     *
     * @return a vector with resulting revisions
     */
    std::vector<svn_revnum_t>
    update(const Targets & targets,
           const Revision & revision,
           svn_depth_t depth,
           bool depth_is_sticky,
           bool ignore_externals);

    /**
     * Updates the file or directory.
     * @param path The single file or directory to update.
     * @param revision the revision number to checkout.
     *                 Revision::HEAD will checkout the
     *                 latest revision.
     * @param depth the update depth.
     *              The special value svn_depth_unknown fetches whatever was already there (the previous "sticky depth").
     * @param depth_is_sticky If depth != svn_depth_unknown, makes the depth sticky (it will be saved permanently).
     * @param ignore_externals don't affect external destinations.
     * @exception ClientException If libsvn reports an error during the operation.
     *
     * @return The resulting (new) revision of the file or directory.
     */
    svn_revnum_t
    update(const Path & path,
           const Revision & revision,
           svn_depth_t depth,
           bool depth_is_sticky,
           bool ignore_externals);

    /**
     * Retrieves the contents for a specific @a revision of
     * a @a path
     *
     * @param path path of file or directory
     * @param revision revision to retrieve
     * @param peg_revision peg revision to retrieve,
     *        by default is the latest one
     * @exception ClientException If libsvn reports an error during the operation.
     * @return contents of the file
     */
    std::string
    cat(const Path & path,
        const Revision & revision,
        const Revision & peg_revision = Revision::UNSPECIFIED);


    /**
     * Retrieves the contents for a specific @a revision of
     * a @a path and saves it to the destination file @a dstPath.
     *
     * If @a dstPath is empty (""), then this path will be
     * constructed from the temporary directory on this system
     * and the filename in @a path. @a dstPath will still have
     * the file extension from @a path and uniqueness of the
     * temporary filename will be ensured.
     *
     * @param dstPath Filename in which the contents
     *                of the file file will be safed.
     * @param path path or url
     * @param revision
     * @param peg_revision peg revision to retrieve, by default is the latest one
     * @exception ClientException If libsvn reports an error during the operation.
     */
    void
    get(Path & dstPath,
        const Path & path,
        const Revision & revision,
        const Revision & peg_revision = Revision::UNSPECIFIED);


    /**
     * Retrieves the contents for a specific @a revision of
     * a @a path
     *
     * @param path path of file or directory
     * @param revisionStart revision to retrieve
     * @param revisionEnd revision to retrieve
     * @exception ClientException If libsvn reports an error during the operation.
     * @return contents of the file
     */
    AnnotatedFile *
    annotate(const Path & path,
             const Revision & revisionStart,
             const Revision & revisionEnd);

    /**
     * Commits changes to the repository. This usually requires
     * authentication, see Auth.
     *
     * @since 0.14 see additional information about the commit
     *        in @ref commitInfo
     *
     * @return Returns a long representing the revision. It returns a
     *         -1 if the revision number is invalid.
     * @param targets files to commit.
     * @param message log message.
     * @param recurse whether the operation should be done recursively.
     * @param keep_locks whether to preserve locks or to release them after commit
     * @exception ClientException If libsvn reports an error during the operation.
     */
    svn_revnum_t
    commit(const Targets & targets,
           const char * message,
           bool recurse,
           bool keep_locks = false);

    /**
     * Copies a versioned file with the history preserved.
     * @exception ClientException If libsvn reports an error during the operation.
     */
    void
    copy(const Path & srcPath,
         const Revision & srcRevision,
         const Path & destPath);

    /**
     * Moves or renames a file.
     * @exception ClientException If libsvn reports an error during the operation.
     */
    void
    move(const Path & srcPath,
         const Revision & srcRevision,
         const Path & destPath,
         bool force);

    /**
     * Creates a directory directly in a repository or creates a
     * directory on disk and schedules it for addition. If <i>path</i>
     * is a URL then authentication is usually required, see Auth.
     *
     * @param path
     * @exception ClientException If libsvn reports an error during the operation.
     */
    void
    mkdir(const Path & path);

    /**
     * Creates one or several directories directly in a repository or
     * creates one or several directories on disk and schedules them
     * for addition. If <i>path</i>
     * is a URL then authentication is usually required, see Auth.
     *
     * @param path
     * @exception ClientException If libsvn reports an error during the operation.
     */
    void
    mkdir(const Targets & targets);

    /**
     * Recursively cleans up a local directory, finishing any
     * incomplete operations, removing lockfiles, etc.
     * @param path a local directory.
     * @exception ClientException If libsvn reports an error during the operation.
     */
    void
    cleanup(const Path & path);

    /**
     * Recursively cleans up a local directory, finishing any
     * incomplete operations, removing lockfiles, etc.
     * New method for Subversion 1.9 and higher.
     * @param path absolute path a local directory.
     * @param breakLocks Break existing working copy locks at or below the @a path
     * @param fixTimestamps Fix recorded timestamps for unmodified files
     * @param vacuumPristines Remove unreferenced files in the pristine store
     * @param includeExternals Recurse into externals and clean them up as well
     * @exception ClientException If libsvn reports an error during the operation.
     */
    void
    cleanup(const Path & path,
            bool breakLocks, bool fixTimestamps, bool vacuumPristines,
            bool includeExternals);

    /**
     * Recursively removes unversioned and/or ignored files from a local directory.
     * New method for Subversion 1.9 and higher.
     * @param path absolute path a local directory.
     * @param removeUnversioned Remove all unversioned files.
     * @param removeIgnored Remove all ignored files.
     * @param fixTimestamps Fix recorded timestamps for unmodified files
     * @param vacuumPristines Remove unreferenced files in the pristine store
     * @param includeExternals Recurse into externals and clean them up as well
     * @exception ClientException If libsvn reports an error during the operation.
     */
    void
    vacuum(const Path &path,
           bool removeUnversioned, bool removeIgnored,
           bool fixTimestamps, bool vacuumPristines,
           bool includeExternals);

    /**
     * Upgrades an entire working copy to the newest format supported
     * by the used SVN library.
     * It is necessary to upgrade working copies before performing any other
     * operations on them, if they were checked out with an older SVN version.
     * @param path Path to the working copy root directory.
     * @exception ClientException If libsvn reports an error during the operation.
     */
    void
    upgrade(const Path &path);

    /**
     * Removes the 'conflicted' state on a file.
     * @exception ClientException If libsvn reports an error during the operation.
     */
    void
    resolved(const Path & path, bool recurse);

    /**
     * Export into file or directory TO_PATH from local or remote FROM_PATH
     * @param from_path path to import
     * @param to_path where to import
     * @param revision revision of files in source repository or working copy
     * @param peg_revision
     * @param overwrite overwrite existing files in to_path
     * @param ignore_externals whether to ignore external sources in from_path
     * @param depth recursion depth
     * @param native_eol which EOL to use when exporting, usually different for
     * different OSs
     * @exception ClientException If libsvn reports an error during the operation.
     */
    void
    doExport(const Path & from_path,
             const Path & to_path,
             const Revision & revision,
             bool overwrite = false,
             const Revision & peg_revision = Revision::UNSPECIFIED,
             bool ignore_externals = false,
             svn_depth_t depth = svn_depth_infinity,
             const char * native_eol = NULL);

    /**
     * Update local copy to mirror a new url. This excapsulates the
     * svn_client_switch() client method.
     * @exception ClientException If libsvn reports an error during the operation.
     */
    svn_revnum_t
    doSwitch(const Path & path, const char * url,
             const Revision & revision,
             bool recurse);

    /**
     * Import file or directory PATH into repository directory URL at
     * head.  This usually requires authentication, see Auth.
     * @param path path to import
     * @param url
     * @param message log message.
     * @param recurse
     * @exception ClientException If libsvn reports an error during the operation.
     */
    void
    import(const Path & path,
           const char * url,
           const char * message,
           bool recurse);
    void
    import(const Path & path,
           const Path & url,
           const char * message,
           bool recurse);


    /**
     * Merge changes from two paths into a new local path.
     * @exception ClientException If libsvn reports an error during the operation.
     */
    void
    merge(const Path & path1, const Revision & revision1,
          const Path & path2, const Revision & revision2,
          const Path & localPath, bool force,
          bool recurse,
          bool notice_ancestry = false,
          bool dry_run = false);


    /**
     * retrieve information about the given path
     * or URL
     *
     * @see Client::status
     * @see Info
     *
     * @param pathOrUrl
     * @param pegRevision
     * @param revision
     * @param recurse
     * @exception ClientException If libsvn reports an error during the operation.
     */
    InfoVector
    info(const Path & pathOrUrl,
         bool recurse=false,
         const Revision & revision = Revision::UNSPECIFIED,
         const Revision & pegRevision = Revision::UNSPECIFIED);


    /**
     * Retrieve log information for the given path
     * Loads the log messages result set. The first
     * entry  is the youngest revision.
     *
     * You can use the constants Revision::START and
     * Revision::HEAD
     *
     * @param path
     * @param revisionStart
     * @param revisionEnd
     * @param discoverChangedPaths
     * @param strictNodeHistory
     * @exception ClientException If libsvn reports an error during the operation.
     * @return a vector with log entries
     */
    const LogEntries *
    log(const char * path,
        const Revision & revisionStart,
        const Revision & revisionEnd,
        bool discoverChangedPaths = false,
        bool strictNodeHistory = true);

    /**
     * Retrieve log information for the given path
     * Loads the log messages result set. The first
     * entry  is the youngest revision.
     * Can limit the number of log entries returned.
     *
     * You can use the constants Revision::START and
     * Revision::HEAD
     *
     * @param path
     * @param revisionStart
     * @param revisionEnd
     * @param limit
     * @param discoverChangedPaths
     * @param strictNodeHistory
     * @exception ClientException If libsvn reports an error during the operation.
     * @return a vector with log entries
     */
    const LogEntries *
    log(const char * path,
        const Revision & revisionStart,
        const Revision & revisionEnd,
        const int limit,
        bool discoverChangedPaths = false,
        bool strictNodeHistory = true);

    /**
     * Produce diff output which describes the delta between
     * @a path/@a revision1 and @a path/@a revision2. @a path
     * can be either a working-copy path or a URL.
     *
     * A ClientException will be thrown if either @a revision1 or
     * @a revision2 has an `unspecified' or unrecognized `kind'.
     *
     * @param tmpPath prefix for a temporary directory needed by diff.
     * Filenames will have ".tmp" and similar added to this prefix in
     * order to ensure uniqueness.
     * @param path path of the file.
     * @param revision1 one of the revisions to check.
     * @param revision2 the other revision.
     * @param recurse whether the operation should be done recursively.
     * @param ignoreAncestry whether the files will be checked for
     * relatedness.
     * @param noDiffDeleted if true, no diff output will be generated
     * on deleted files.
     * @return delta between the files
     * @exception ClientException If libsvn reports an error during the operation.
     */
    std::string
    diff(const Path & tmpPath, const Path & path,
         const Revision & revision1, const Revision & revision2,
         const bool recurse, const bool ignoreAncestry,
         const bool noDiffDeleted);

    /**
     * Produce diff output which describes the delta between
     * @a path1/@a revision1 and @a path2/@a revision2. @a path1,
     * @a path2 can be either a working-copy path or a URL.
     *
     * A ClientException will be thrown if either @a revision1 or
     * @a revision2 has an `unspecified' or unrecognized `kind'.
     *
     * @param tmpPath prefix for a temporary directory needed by diff.
     * Filenames will have ".tmp" and similar added to this prefix in
     * order to ensure uniqueness.
     * @param path1 path of the first file corresponding to @a revision1.
     * @param path2 path of the first file corresponding to @a revision2.
     * @param revision1 one of the revisions to check.
     * @param revision2 the other revision.
     * @param recurse whether the operation should be done recursively.
     * @param ignoreAncestry whether the files will be checked for
     * relatedness.
     * @param noDiffDeleted if true, no diff output will be generated
     * on deleted files.
     * @return delta between the files
     * @exception ClientException If libsvn reports an error during the operation.
     */
    std::string
    diff(const Path & tmpPath, const Path & path1, const Path & path2,
         const Revision & revision1, const Revision & revision2,
         const bool recurse, const bool ignoreAncestry,
         const bool noDiffDeleted);

    /**
     * Produce diff output which describes the delta of
     * @a path/@a pegRevision between @a revision1 and @a revision2.
     * @a path can be either a working-copy path or a URL.
     *
     * A ClientException will be thrown if either @a revision1 or
     * @a revision2 has an `unspecified' or unrecognized `kind'.
     *
     * @param tmpPath prefix for a temporary directory needed by diff.
     * Filenames will have ".tmp" and similar added to this prefix in
     * order to ensure uniqueness.
     * @param path path of the file.
     * @param pegRevision the peg revision to identify the path.
     * @param revision1 one of the revisions to check.
     * @param revision2 the other revision.
     * @param recurse whether the operation should be done recursively.
     * @param ignoreAncestry whether the files will be checked for
     * relatedness.
     * @param noDiffDeleted if true, no diff output will be generated
     * on deleted files.
     * @return delta between the files
     * @exception ClientException If libsvn reports an error during the operation.
     */
    std::string
    diff(const Path & tmpPath, const Path & path,
         const Revision & pegRevision, const Revision & revision1,
         const Revision & revision2, const bool recurse,
         const bool ignoreAncestry, const bool noDiffDeleted);

    /**
     * lists entries in @a pathOrUrl no matter whether local or
     * repository
     *
     * @param pathOrUrl
     * @param revision
     * @param recurse
     * @return a vector of directory entries, each with
     *         a relative path (only filename)
     * @exception ClientException If libsvn reports an error during the operation.
     */
    DirEntries
    list(const char * pathOrUrl,
         svn_opt_revision_t * revision,
         bool recurse);

    /**
     * lists properties in @a path no matter whether local or
     * repository
     *
     * @param path
     * @param revision
     * @param recurse
     * @exception ClientException If libsvn reports an error during the operation.
     * @return PropertiesList
     */
    PathPropertiesMapList
    proplist(const Path &path,
             const Revision &revision,
             bool recurse = false);

    /**
     * lists one property in @a path no matter whether local or
     * repository
     *
     * @param propName
     * @param path
     * @param revision
     * @param recurse
     * @exception ClientException If libsvn reports an error during the operation.
     * @return PathPropertiesMapList
     */
    PathPropertiesMapList
    propget(const char * propName,
            const Path & path,
            const Revision & revision,
            bool recurse = false);

    /**
     * This method is deprecated, please use
     * @a Property.set
     * set property in @a path no matter whether local or
     * repository
     *
     * @deprecated
     *
     * @param path
     * @param revision
     * @param propName
     * @param propValue
     * @param recurse
     * @param skip_checks
     * @exception ClientException If libsvn reports an error during the operation.
     * @return PropertiesList
     */
    void
    propset(const char * propName,
            const char * propValue,
            const Path & path,
            const Revision & revision,
            bool recurse = false,
            bool skip_checks = true);

    /**
     * delete property in @a path no matter whether local or
     * repository
     *
     * @param propName
     * @param path
     * @param revision
     * @param recurse
     * @exception ClientException If libsvn reports an error during the operation.
     */
    void
    propdel(const char * propName,
            const Path & path,
            const Revision & revision,
            bool recurse = false);


    /**
     * lists revision properties in @a path no matter whether local or
     * repository
     *
     * @param path
     * @param revision
     * @exception ClientException If libsvn reports an error during the operation.
     * @return PropertiesList
     */
    std::pair<svn_revnum_t,PropertiesMap>
    revproplist(const Path & path,
                const Revision & revision);

    /**
     * lists one revision property in @a path no matter whether local or
     * repository
     *
     * @param propName
     * @param path
     * @param revision
     * @exception ClientException If libsvn reports an error during the operation.
     * @return PropertiesList
     */
    std::pair<svn_revnum_t,std::string>
    revpropget(const char * propName,
               const Path & path,
               const Revision & revision);

    /**
     * set revision property in @a path no matter whether local or
     * repository
     *
     * @param propName
     * @param propValue
     * @param path
     * @param revision
     * @param force
     * @exception ClientException If libsvn reports an error during the operation.
     * @return Revision
     */
    svn_revnum_t
    revpropset(const char * propName,
               const char * propValue,
               const Path & path,
               const Revision & revision,
               bool force = false);

    /**
     * delete revision property in @a path no matter whether local or
     * repository
     *
     * @param propName
     * @param path
     * @param revision
     * @param force
     * @exception ClientException If libsvn reports an error during the operation.
     * @return Revision
     */
    svn_revnum_t
    revpropdel(const char * propName,
               const Path & path,
               const Revision & revision,
               bool force = false);


    /**
     * Add a single file into ignore list.
     *
     * @param path path to the file
     * @exception ClientException If libsvn reports an error during the operation.
     * @see svn:ignore property description
     */
    void
    ignore(const Path & path);

    /**
     * Add files into ignore list.
     *
     * @param targets targets to treat as ignored
     * @exception ClientException If libsvn reports an error during the operation.
     * @see svn:ignore property description
     */
    void
    ignore(const Targets & targets);
  private:
    Context * m_context;
    CommitInfo m_commitInfo;

    /**
     * disallow assignment operator
     */
    Client & operator= (const Client &);

    /**
     * disallow copy constructor
     */
    Client(const Client &);
  };
}

#endif
/* -----------------------------------------------------------------
 * local variables:
 * eval: (load-file "../../rapidsvn-dev.el")
 * end:
 */
