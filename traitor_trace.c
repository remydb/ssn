/* 
 * Traitor Trace VFS module. 
 *
 * Copyright (C) Rene Klomp
 * Copyright (C) Stefan Plug
 * Copyright (C) Remy de Boer
 * Copyright (C) Thijs Rozekrans
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "includes.h"
#include "smbd/proto.h"

#include <fcntl.h>
#include <vfs.h>
#include <auth.h>

typedef struct traitor_trace_file_map {
	struct traitor_trace_file_map *next;
	char *filename;	
	char *tempname;
} traitor_trace_file_map_entry;

traitor_trace_file_map_entry *traitor_trace_file_map_list=NULL;

traitor_trace_file_map_entry *traitor_trace_file_map_insert(char *filename, char *tempname){
	DEBUG(1, ("traitor_trace_file_map_insert: %s, %s\n", filename, tempname));
	traitor_trace_file_map_entry *node;
	if(!(node=malloc(sizeof(traitor_trace_file_map_entry)))) return NULL;
	node->filename=filename;
	node->tempname=tempname;
	if(traitor_trace_file_map_list==NULL){
		node->next=NULL;
	}
	else{
		node->next=traitor_trace_file_map_list;
	}
	traitor_trace_file_map_list = node;
	DEBUG(1, ("traitor_trace_file_map_insert: %s\n",traitor_trace_file_map_list->tempname));
	return node;
}


void traitor_trace_file_map_remove(char *tempname) {
	if(traitor_trace_file_map_list == NULL) return;

	traitor_trace_file_map_entry *list = traitor_trace_file_map_list;
	while(list->next && list->tempname!=tempname) list=list->next;
	if(list->next) {
		traitor_trace_file_map_entry *hit = list->next;
		list->next=list->next->next;
		free(hit);
	}
}

traitor_trace_file_map_entry *traitor_trace_file_map_find(char *filename){
	if(traitor_trace_file_map_list == NULL) return NULL;

	traitor_trace_file_map_entry *list = traitor_trace_file_map_list;
	while(list->next && list->filename!=filename) list=list->next;
	if(list->next) {
		return list->next;
	}
	return NULL;
}

/*
 * Traitor Trace VFS Functions
 */

static int traitor_trace_open(vfs_handle_struct *handle, struct smb_filename *smb_fname,
		     files_struct *fsp, int flags, mode_t mode)
{
	char *user,*filename,*command;
	const char *script;
	FILE *fp;
	char line[128];
	unsigned int len;


	//prepare some variables
	user = handle->conn->session_info->sanitized_username;
	filename = smb_fname->base_name;
	script = lp_parm_const_string(SNUM(handle->conn),"traitor_trace", "script", "none");
	
	//check to see if the file exists
	SMB_STRUCT_STAT buf;
	if (sys_stat(filename,&buf,0) != 0)
		return SMB_VFS_NEXT_OPEN(handle, smb_fname, fsp, flags, mode);

	//run command and open output to read
	DEBUG(1, ("traitor_trace_open %s by %s\n", filename, user));
	asprintf(&command,"%s %s \"%s\"", script, user, filename);
	fp = popen(command, "r");

	//read first line. This sould contain the temporary file
	fgets(line, sizeof line, fp);
	
	//remove the eol char
	len = strlen(line);
	if (line[len - 1] == '\n')
	    line[len - 1] = '\0';

	DEBUG(1, ("traitor_trace_open has received tmpfile: %s\n", line));
	fclose(fp);

	traitor_trace_file_map_insert(smb_fname->base_name,line);
	smb_fname->base_name = line;
	return SMB_VFS_NEXT_OPEN(handle, smb_fname, fsp, flags, mode);
}

static int traitor_trace_stat(vfs_handle_struct *handle, struct smb_filename *smb_fname)
{
	traitor_trace_file_map_entry *result = traitor_trace_file_map_find(smb_fname->base_name);
	if(result == NULL)
		DEBUG(1, ("traitor_trace_stat for %s\n", smb_fname->base_name));
	else{
		DEBUG(1, ("traitor_trace_stat for %s\n", result->tempname));
	}
	return SMB_VFS_NEXT_STAT(handle, smb_fname);
}

/* VFS operations structure */

struct vfs_fn_pointers traitor_trace_fns = {
	.open_fn = traitor_trace_open,
	.stat = traitor_trace_stat,
};

NTSTATUS init_samba_module(void)
{
	return smb_register_vfs(SMB_VFS_INTERFACE_VERSION, "traitor_trace", &traitor_trace_fns);
}
