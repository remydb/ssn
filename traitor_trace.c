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
#include <string.h>


typedef struct traitor_trace_file_map {
	struct traitor_trace_file_map *next;
	char *filename;	
	char *tempname;
	int fnum;
} traitor_trace_file_map_entry;

traitor_trace_file_map_entry *traitor_trace_file_map_list=NULL;

traitor_trace_file_map_entry *traitor_trace_file_map_insert(char *filename, char *tempname,int fnum){
	DEBUG(2, ("traitor_trace_file_map_insert: %s, %s\n", filename, tempname));
	
	traitor_trace_file_map_entry *node;
	
	if(!(node=malloc(sizeof(traitor_trace_file_map_entry)))){
		//malloc failed so return NULL
		return NULL;	
	}
	
	//copy data to new list node
	node->filename=strdup(filename);
	node->tempname=strdup(tempname);
	node->fnum=fnum;
	
	//add the node to the list
	if(traitor_trace_file_map_list==NULL){
		node->next=NULL;
	}
	else{
		node->next=traitor_trace_file_map_list;
	}
	traitor_trace_file_map_list = node;
	return node;
}


void traitor_trace_file_map_remove(int fnum) {
	//immediately return if list is empty
	if(traitor_trace_file_map_list == NULL) return;

	traitor_trace_file_map_entry *list = traitor_trace_file_map_list;
	traitor_trace_file_map_entry *prev = NULL;

	while(list){
		if(list->fnum==fnum){
			DEBUG(2, ("traitor_trace_file_map_remove: %s\n", list->tempname));

			if(prev==NULL){
				traitor_trace_file_map_list = list->next;
			}
			else{
				prev->next = list->next;
			}
			
			//remove file en free mem
			int stat = unlink(list->tempname);
			if(stat<0){
				DEBUG(2, ("traitor_trace_file_map_remove: %s\n", strerror(errno)));
			}
			free(list->filename);
			free(list->tempname);
			free(list);
			return;
		}
		prev = list;
		list = list->next;
	}
	return;
}

traitor_trace_file_map_entry *traitor_trace_file_map_find(char *filename){
	//immediately return if list is empty
	if(traitor_trace_file_map_list == NULL) return NULL;

	traitor_trace_file_map_entry *list = traitor_trace_file_map_list;

	while(list){
		if(strcmp(list->filename,filename)==0){
			DEBUG(2, ("traitor_trace_file_map_find: Found %s\n", list->tempname));
			return list;
		}
		list = list->next;
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
	char *line = NULL;
    size_t len = 0;


	//prepare some variables
	user = handle->conn->session_info->sanitized_username;
	filename = smb_fname->base_name;
	script = lp_parm_const_string(SNUM(handle->conn),"traitor_trace", "script", "none");
	
	//check to see if the file exists
	SMB_STRUCT_STAT buf;
	if (sys_stat(filename,&buf,0) != 0)
		return SMB_VFS_NEXT_OPEN(handle, smb_fname, fsp, flags, mode);

	//run command and open output to read
	DEBUG(1, ("traitor_trace_open %s/%s by %s\n", handle->conn->origpath, filename, user));
	if(!asprintf(&command,"python %s %s/%s \"%s\"", script, handle->conn->origpath, filename, user)){
		//asprintf failed
		return -1;
	}
	fp = popen(command, "r");

	//read first line. This should contain the temporary file
	if(getline(&line, &len, fp)<0){
		//fgets failed
		return -1;
	}
	
	//remove the eol char
	len = strlen(line);
	if (line[len - 1] == '\n')
	    line[len - 1] = '\0';

	DEBUG(1, ("traitor_trace_open line size %d\n",(int)strlen(line)));
	if(len>strlen(filename)){
		DEBUG(1, ("traitor_trace_open has received tmpfile: %s\n", line));
		fclose(fp);

		traitor_trace_file_map_insert(smb_fname->base_name,line,fsp->fnum);
		smb_fname->base_name = line;
	}
	return SMB_VFS_NEXT_OPEN(handle, smb_fname, fsp, flags, mode);
}

static int traitor_trace_close(vfs_handle_struct *handle, files_struct *fsp)
{
	DEBUG(1, ("traitor_trace_close: %d\n", fsp->fnum));
	traitor_trace_file_map_remove(fsp->fnum);
	return SMB_VFS_NEXT_CLOSE(handle, fsp);
}

static int traitor_trace_stat(vfs_handle_struct *handle, struct smb_filename *smb_fname)
{
	traitor_trace_file_map_entry *result = traitor_trace_file_map_find(smb_fname->base_name);
	if(result != NULL){
		smb_fname->base_name = result->tempname;
		DEBUG(2, ("traitor_trace_stat for %s\n", result->tempname));
	}
	return SMB_VFS_NEXT_STAT(handle, smb_fname);
}

/* VFS operations structure */

struct vfs_fn_pointers traitor_trace_fns = {
	.open_fn = traitor_trace_open,
	.close_fn = traitor_trace_close,
	.stat = traitor_trace_stat,
};

NTSTATUS init_samba_module(void) {
	return smb_register_vfs(SMB_VFS_INTERFACE_VERSION, "traitor_trace", &traitor_trace_fns);
}
