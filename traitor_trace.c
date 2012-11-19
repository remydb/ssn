/* 
 * Skeleton VFS module.  Implements passthrough operation of all VFS
 * calls to disk functions.
 *
 * Copyright (C) Tim Potter, 1999-2000
 * Copyright (C) Alexander Bokovoy, 2002
 * Copyright (C) Stefan (metze) Metzmacher, 2003
 * Copyright (C) Jeremy Allison 2009
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *  
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */


#include "includes.h"
#include "smbd/proto.h"

#include <fcntl.h>
#include <vfs.h>
#include <auth.h>

static int traitor_trace_open(vfs_handle_struct *handle, struct smb_filename *smb_fname,
		     files_struct *fsp, int flags, mode_t mode)
{
	char *user,*filename,*command;

	//prepare some variables
	user = handle->conn->session_info->sanitized_username;
	filename = smb_fname->base_name;
	
	//check to see if the file exists
	SMB_STRUCT_STAT buf;
	if (sys_stat(filename,&buf,0) != 0)
		return SMB_VFS_NEXT_OPEN(handle, smb_fname, fsp, flags, mode);

	//run command and pipe output back to the smbd
	DEBUG(1, ("traitor_trace_open %s by %s\n", filename, user));
	asprintf(&command,"/var/samba/mytest %s \"%s\"", user, filename);
	return sys_popen(command);
}


/* VFS operations structure */

struct vfs_fn_pointers traitor_trace_fns = {
	.open_fn = traitor_trace_open,
};

NTSTATUS init_samba_module(void)
{
	return smb_register_vfs(SMB_VFS_INTERFACE_VERSION, "traitor_trace", &traitor_trace_fns);
}
