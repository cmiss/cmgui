/*******************************************************************************
FILE : filedir.c

LAST MODIFIED : 24 March 2009

DESCRIPTION :
Routines for opening files using wx widgets.
???DB.  Return sometimes doesn't work for writing ?  This is because Cancel is
the default button.
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (1) */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "general/debug.h"
#include "user_interface/filedir.h"
#include "general/message.h"
#include "user_interface/user_interface.h"
#include "general/mystring.h"
#include "command/command.h"
#if defined (WX_USER_INTERFACE)
#include "wx/wx.h"
#include <wx/tglbtn.h>
#include "wx/xrc/xmlres.h"
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#endif /* defined (WX_USER_INTERFACE)*/

/*
Code switchs
------------
*/
/* if a file selection dialog changes directory then when the dialog is opened
	again it starts in the changed directory */
#define REMEMBER_LAST_DIRECTORY_FOR_FILE_SELECTION

/*
Global functions
----------------
*/
struct File_open_data *create_File_open_data(const char *filter_extension,
	enum File_type type,File_operation operation,void *arguments,
	 int allow_direct_to_printer,struct User_interface *user_interface
#if defined(WX_USER_INTERFACE)
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
																						 )
/*******************************************************************************
LAST MODIFIED : 3 June 1999

DESCRIPTION :
This function allocates memory for file open data and initializes the fields to
the specified values.  It returns a pointer to the created structure if
successful and NULL if unsuccessful.
==============================================================================*/
{
	struct File_open_data *file_open_data;
#if defined (WIN32_USER_INTERFACE)
	const char *temp_filter_extension;
//#elif defined(WX_USER_INTERFACE)
//	char *shell_title,*temp_string;
#endif /* defined (SWITCH_USER_INTERFACE) */
	ENTER(create_File_open_data);
	/* check arguments */
	if (user_interface)
	{
		if (ALLOCATE(file_open_data,struct File_open_data,1))
		{
			if (filter_extension)
			{
				if (ALLOCATE(file_open_data->filter_extension,char,
					strlen(filter_extension)+1))
				{
					strcpy(file_open_data->filter_extension,filter_extension);
				}
				else
				{
					display_message(ERROR_MESSAGE,
			"create_File_open_data.  Could not allocate memory for filter extension");
				}
			}
			else
			{
				file_open_data->filter_extension=(char *)NULL;
			}
			file_open_data->type=type;
			file_open_data->operation=operation;
			file_open_data->arguments=arguments;
			file_open_data->cancel_operation=(File_cancel_operation)NULL;
			file_open_data->cancel_arguments=NULL;
			file_open_data->allow_direct_to_printer=allow_direct_to_printer;
			file_open_data->user_interface=user_interface;
#if defined(WX_USER_INTERFACE)
			file_open_data->execute_command=execute_command;
#endif /* defined (WX_USER_INTERFACE) */
			file_open_data->file_name=(char *)NULL;
#if defined (WIN32_USER_INTERFACE)
			(file_open_data->open_file_name).lStructSize=sizeof(OPENFILENAME);
			(file_open_data->open_file_name).hwndOwner=(HWND)NULL;
			(file_open_data->open_file_name).hInstance=(HINSTANCE)NULL;
			if (temp_filter_extension=filter_extension)
			{
				if ('.'==temp_filter_extension[0])
				{
					temp_filter_extension++;
				}
				if (ALLOCATE((file_open_data->open_file_name).lpstrFilter,char,
					2*strlen(temp_filter_extension)+11))
				{
					/*???DB.  Unicode ? */
					strcpy((char *)((file_open_data->open_file_name).lpstrFilter),
						temp_filter_extension);
					strcat((char *)((file_open_data->open_file_name).lpstrFilter),
						" files");
					strcpy((char *)(((file_open_data->open_file_name).lpstrFilter)+
						(strlen(temp_filter_extension)+7)),"*.");
					strcat((char *)(((file_open_data->open_file_name).lpstrFilter)+
						(strlen(temp_filter_extension)+7)),temp_filter_extension);
					((char *)((file_open_data->open_file_name).lpstrFilter))
						[2*strlen(temp_filter_extension)+10]='\0';
				}
			}
			else
			{
				(file_open_data->open_file_name).lpstrFilter=(char *)NULL;
			}
			(file_open_data->open_file_name).lpstrCustomFilter=(char *)NULL;
			(file_open_data->open_file_name).nMaxCustFilter=0;
			(file_open_data->open_file_name).nFilterIndex=1;
			(file_open_data->open_file_name).nMaxFile=256;
			if (ALLOCATE((file_open_data->open_file_name).lpstrFile,char,
				(file_open_data->open_file_name).nMaxFile))
			{
				((file_open_data->open_file_name).lpstrFile)[0]='\0';
				(file_open_data->open_file_name).lpstrFileTitle=(char *)NULL;
				(file_open_data->open_file_name).nMaxFileTitle=0;
				(file_open_data->open_file_name).lpstrInitialDir=(char *)NULL;
				(file_open_data->open_file_name).lpstrTitle=(char *)NULL;
				(file_open_data->open_file_name).Flags=OFN_EXPLORER|OFN_OVERWRITEPROMPT;
				(file_open_data->open_file_name).nFileOffset=0;
				(file_open_data->open_file_name).nFileExtension=0;
				if (ALLOCATE((file_open_data->open_file_name).lpstrDefExt,char,
					strlen(filter_extension)+1))
				{
					strcpy((char *)((file_open_data->open_file_name).lpstrDefExt),
						filter_extension);
				}
					/*???DB.  Only the first 3 characters are appended ! */
				(file_open_data->open_file_name).lCustData=(DWORD)NULL;
				(file_open_data->open_file_name).lpfnHook=(LPOFNHOOKPROC)NULL;
				(file_open_data->open_file_name).lpTemplateName=(char *)NULL;
			}
			else
			{
				DEALLOCATE(file_open_data->filter_extension);
				DEALLOCATE((file_open_data->open_file_name).lpstrFilter);
				DEALLOCATE(file_open_data);
				display_message(ERROR_MESSAGE,
					"create_File_open_data.  Could not allocate memory for lpstrFile");
			}
#endif /* defined (WIN32_USER_INTERFACE) */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_File_open_data.  Could not allocate memory for file_open_data");
		}
	}
	else
	{
		file_open_data=(struct File_open_data *)NULL;
	}
	LEAVE;

	return (file_open_data);
} /* create_File_open_data */

int destroy_File_open_data(struct File_open_data **file_open_data)
/*******************************************************************************
LAST MODIFIED : 20 April 1997

DESCRIPTION :
This function frees the memory associated with the fields of <**file_open_data>,
frees the memory for <**file_open_data> and changes <*file_open_data> to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(destroy_File_open_data);
	return_code=1;
	if (*file_open_data)
	{
		if ((*file_open_data)->filter_extension)
		{
			 DEALLOCATE((*file_open_data)->filter_extension);
		}
		if ((*file_open_data)->file_name)
		{
			DEALLOCATE((*file_open_data)->file_name);
		}
		DEALLOCATE(*file_open_data);
		*file_open_data=(struct File_open_data *)NULL;
	}
	LEAVE;

	return (return_code);
} /* destroy_File_open_data */

void open_file_and_read(
#if defined (WIN32_USER_INTERFACE) || defined (WX_USER_INTERFACE)
	struct File_open_data *file_open_data
#endif /* defined (WX_USER_INTERFACE) */

	)
/*******************************************************************************
LAST MODIFIED : 24 March 2009

DESCRIPTION :
Expects a pointer to a File_open_data structure as the <client_data>.  Displays
a list of the file names matching the <filter>.  After the user selects a file
name the <file_operation> is performed on the file with the <arguments>.
==============================================================================*/
{
#if defined (WX_USER_INTERFACE)
	const char *shell_title;
	char *allocated_shell_title,*temp_string,*extension;
	const char *filename;
	int length;
#endif /* defined (WX_USER_INTERFACE) */
#if defined (WIN32_USER_INTERFACE)
	char *temp_str;
	int length,retry;
#endif /* defined (WIN32_USER_INTERFACE) */

	ENTER(open_file_and_read);
#if defined (WIN32_USER_INTERFACE)
	if (file_open_data)
	{
		retry=0;
		do
		{
			if (TRUE==GetOpenFileName(&(file_open_data->open_file_name)))
			{
				/* keep the current directory */
				length=0;
				if (temp_str=strrchr((file_open_data->open_file_name).lpstrFile,'\\'))
				{
					length=(temp_str-(file_open_data->open_file_name).lpstrFile)+1;
				}
				if ((0<length)&&REALLOCATE(temp_str,
					(file_open_data->open_file_name).lpstrInitialDir,char,length+1))
				{
					(file_open_data->open_file_name).lpstrInitialDir=temp_str;
					strncpy(temp_str,(file_open_data->open_file_name).lpstrFile,length);
					temp_str[length]='\0';
				}
				if (file_open_data->operation)
				{
					if ((file_open_data->operation)((file_open_data->open_file_name).
						lpstrFile,file_open_data->arguments))
					{
						retry=0;
					}
					else
					{
						retry=1;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"open_file_and_read.  No file operation");
					retry=0;
				}
			}
			else
			{
				retry=0;
			}
		} while (retry);
	}
	else
	{
		display_message(ERROR_MESSAGE,"open_file_and_read.  No file_open_data");
	}
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
	temp_string=(char *)NULL;
	extension=(char *)NULL;
	allocated_shell_title=(char *)NULL;
	switch (file_open_data->type)
	{
		case REGULAR:
		{
			shell_title="Specify a file";
			if (file_open_data->filter_extension)
			{
				if (ALLOCATE(allocated_shell_title,char,strlen(shell_title)+
					strlen(file_open_data->filter_extension)+2))
				{
					strcpy(allocated_shell_title,"Specify a ");
					strcat(allocated_shell_title,file_open_data->filter_extension);
					strcat(allocated_shell_title," file");
					shell_title=allocated_shell_title;
				}
				const char *wildcard_extension = wxT("All files (*.*)|*.*|");
				if (ALLOCATE(extension,char,
						strlen(file_open_data->filter_extension)*2+strlen(wildcard_extension)+4))
				{
					strcpy(extension,wildcard_extension);
					strcat(extension,wxT("*"));
					strcat(extension,file_open_data->filter_extension);
					strcat(extension,wxT("|*"));
					strcat(extension,file_open_data->filter_extension);
				}
			}
		} break;
		case DIRECTORY:
		{
			shell_title="Specify a directory";
		} break;
		default:
		{
			shell_title=(char *)NULL;
		} break;
	}
	//	wxFileDialog *ReadData = new wxFileDialog ((wxWindow *)NULL,wxString::FromAscii(shell_title),wxT(""),wxT(""),
	//											   wxT("*.com;*.exnode"),wxOPEN|wxFILE_MUST_EXIST,wxDefaultPosition);
	wxFileDialog *ReadData = new wxFileDialog ((wxWindow *)NULL,wxString::FromAscii(shell_title),wxT(""),wxT(""),
											   extension,wxFD_OPEN|wxFD_FILE_MUST_EXIST,wxDefaultPosition);
	if (ReadData->ShowModal() == wxID_OK)
	{
		 wxString file_name=ReadData->GetPath();
		 file_open_data->file_name=duplicate_string(file_name.mb_str(wxConvUTF8));
#if defined (WIN32_SYSTEM)
		 char *drive_name = NULL;
		 char *temp_directory_name,*directory_name, *temp_name;
		 int lastlength;
		 filename = file_open_data->file_name;
		 directory_name = NULL;
		 const char *first = strchr(filename, '\\');
		 const char *last = strrchr(filename, '\\');
		 lastlength = last - filename +1;
		 length = first - filename +1;
		 if ((length>0))
		 {
				if (ALLOCATE(drive_name,char,length))
				{
					 strncpy(drive_name,file_name,length);
					 drive_name[length-1]='\0';
					 if (ALLOCATE(temp_string,char,length+13))
					 {
							strcpy(temp_string, "set dir \/d ");
							strcat(temp_string, drive_name);
							strcat(temp_string, "\\");
							temp_string[length+12]='\0';
							Execute_command_execute_string(file_open_data->execute_command,temp_string);
							DEALLOCATE(temp_string);
					 }
					 DEALLOCATE(drive_name);
					 if (length == lastlength)
					 {
							temp_name = const_cast<char *>(&filename[lastlength]);
					 }
				}
				if (lastlength>length)
				{
					 if (ALLOCATE(temp_directory_name,char,lastlength+1))
					 {
							strncpy(temp_directory_name,filename,lastlength);
							if (ALLOCATE(directory_name,char,lastlength-length+1))
							{
								strncpy(directory_name,&temp_directory_name[length-1],lastlength-length);
								directory_name[lastlength-length] = '\0';
								if (ALLOCATE(temp_string,char,lastlength-length+11))
								{
										strcpy(temp_string, "set dir \'");
										strcat(temp_string, directory_name);
										strcat(temp_string, "\'");
										temp_string[lastlength-length+10]='\0';
										Execute_command_execute_string(file_open_data->execute_command,temp_string);
										temp_name=const_cast<char *>(&filename[lastlength]);
										if (file_open_data->operation)
										{
											(file_open_data->operation)((temp_name)
														,file_open_data->arguments);
										}
										DEALLOCATE(temp_string);
								}
								DEALLOCATE(directory_name);
							}
							DEALLOCATE(temp_directory_name);
					 }
				}
				else if (lastlength==length)
				{
					temp_name = const_cast<char *>(&filename[lastlength]);
				}
				if (file_open_data->operation)
					(file_open_data->operation)((temp_name), file_open_data->arguments);
		 }
#else /*defined (WIN32_SYSTEM)*/
		 if (file_open_data->operation)
		 {
				(file_open_data->operation)((file_open_data->file_name)
							,file_open_data->arguments);
		 }
		 char *old_directory = NULL;
		 char *old_directory_name = NULL;
		 const char *last;
		char *pathname;
		 int return_code=1;
		 old_directory = (char *)malloc(4096);
		 if (!getcwd(old_directory, 4096))
		 {
		   // Unable to read old directory so just set it to a empty string
		   old_directory[0] = 0;
		 }
		 length = strlen(old_directory);
		 filename = file_open_data->file_name;
		 if ((ALLOCATE(old_directory_name,char,length+2)) && old_directory !=NULL)
		 {
				strcpy(old_directory_name, old_directory);
				strcat(old_directory_name,"/");
				old_directory_name[length+1]='\0';
		 }
		 else
		 {
				return_code=0;
		 }
		 if (strcmp(file_open_data->filter_extension, ".com") != 0)
				/* Set the directory if the file extension is not .com.
					 if the file extension is .com, the directory will be set in the
					 comfile.com */
		 {
				last = strrchr(filename, '/');
				if (last != NULL)
				{
					 length = last-filename+1;
					 pathname = NULL;
					 if (ALLOCATE(pathname,char,length+1))
					 {
							strncpy(pathname,filename,length);
							pathname[length]='\0';
							if (return_code && (strcmp (old_directory_name,pathname) != 0))
							{
								 make_valid_token(&pathname);
								 length = strlen(pathname);
								 if (ALLOCATE(temp_string,char,length+9))
								 {
										strcpy(temp_string, "set dir ");
										strcat(temp_string, pathname);
										temp_string[length+8]='\0';
										Execute_command_execute_string(file_open_data->execute_command,temp_string);
								 }
								 if (temp_string)
								 {
										DEALLOCATE(temp_string);
								 }
							}
					 }
					 if (pathname)
					 {
							DEALLOCATE(pathname);
					 }
				}
		 }
		 if (old_directory_name)
		 {
				DEALLOCATE(old_directory_name);
		 }
		 if (old_directory)
		 {
				free(old_directory);
		 }
#endif /* !defined (WIN32_SYSTEM)*/
	}
	DEALLOCATE(allocated_shell_title);
	DEALLOCATE(extension);
	DEALLOCATE(temp_string);
#endif /* defined (WX_USER_INTERFACE) */
	LEAVE;
} /* open_file_and_read */

void open_file_and_write(
#if defined (WIN32_USER_INTERFACE)
	struct File_open_data *file_open_data
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
	struct File_open_data *file_open_data
#endif /* defined (WX_USER_INTERFACE) */
	)
/*******************************************************************************
LAST MODIFIED : 9 June 2003

DESCRIPTION :
Expects a pointer to a File_open_data structure as the <client_data>.
Prompts the user for the name of file, omitting the <extension>, to write to.
The <file_operation> with the <user_arguments> and the output directed to the
specified file.
==============================================================================*/
{
#if defined (WX_USER_INTERFACE)
//	char *temp_str;
	const char *shell_title;
	char *temp_string,*extension;
#endif /* defined (WX_USER_INTERFACE) */
#if defined (WIN32_USER_INTERFACE)
	char *temp_str;
	int length,retry;
#endif /* defined (WIN32_USER_INTERFACE) */

	ENTER(open_file_and_write);
#if defined (WIN32_USER_INTERFACE)
	if (file_open_data)
	{
		retry=0;
		do
		{
			if (TRUE==GetSaveFileName(&(file_open_data->open_file_name)))
			{
				/* keep the current directory */
				length=0;
				if (temp_str=strrchr((file_open_data->open_file_name).lpstrFile,'\\'))
				{
					length=(temp_str-(file_open_data->open_file_name).lpstrFile)+1;
				}
				if ((0<length)&&REALLOCATE(temp_str,
					(file_open_data->open_file_name).lpstrInitialDir,char,length+1))
				{
					(file_open_data->open_file_name).lpstrInitialDir=temp_str;
					strncpy(temp_str,(file_open_data->open_file_name).lpstrFile,length);
					temp_str[length]='\0';
				}
				if (file_open_data->operation)
				{
					if ((file_open_data->operation)((file_open_data->open_file_name).
						lpstrFile,file_open_data->arguments))
					{
						retry=0;
					}
					else
					{
						retry=1;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"open_file_and_read.  No file operation");
					retry=0;
				}
			}
			else
			{
				retry=0;
			}
		} while (retry);
	}
	else
	{
		display_message(ERROR_MESSAGE,"open_file_and_write.  No file_open_data");
	}
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
	temp_string=(char *)NULL;
	extension = (char *)NULL;
	switch (file_open_data->type)
	{
		case REGULAR:
		{
			shell_title="Specify a file";
			if (file_open_data->filter_extension)
			{
				if (ALLOCATE(temp_string,char,strlen(shell_title)+
						strlen(file_open_data->filter_extension)+2))
				{
					strcpy(temp_string,"Specify a ");
					strcat(temp_string,file_open_data->filter_extension);
					strcat(temp_string," file");
					shell_title=temp_string;
				}
				const char *wildcard_extension = "All files (*.*)|*.*|";
				if (ALLOCATE(extension,char,
						strlen(file_open_data->filter_extension)*2+strlen(wildcard_extension)+4))
				{
					strcpy(extension,wildcard_extension);
					strcat(extension,"(*");
					strcat(extension,file_open_data->filter_extension);
					strcat(extension,"*");
					strcat(extension,file_open_data->filter_extension);
				}
			}
		} break;
		case DIRECTORY:
		{
			shell_title="Specify a directory";
		} break;
		default:
		{
			shell_title=(char *)NULL;
		} break;
	}
	wxFileDialog *SaveData = new wxFileDialog ((wxWindow *)NULL,wxString::FromAscii(shell_title),wxT(""),wxT(""),
		wxString::FromAscii(extension),wxFD_SAVE|wxFD_OVERWRITE_PROMPT,wxDefaultPosition);
	if (temp_string)
	{
		DEALLOCATE(temp_string);
	}
	if (extension)
	{
		DEALLOCATE(extension);
	}
 if (SaveData->ShowModal() == wxID_OK)
	{
	  wxString file_name=SaveData->GetPath();
		file_open_data->file_name=duplicate_string(file_name.mb_str(wxConvUTF8));
#if defined (WIN32_SYSTEM)
		 char *drive_name = NULL;
		 char *temp_directory_name,*directory_name, *temp_name;
		 int lastlength, length;
		 const char *filename = file_open_data->file_name;
		 const char *first = strchr(filename, '\\');
		 const char *last = strrchr(filename, '\\');
		 lastlength = last - filename +1;
		 length = first - filename +1;
		 if ((length>0))
		 {
				if (ALLOCATE(drive_name,char,length))
				{
					 strncpy(drive_name,file_name,length);
					 drive_name[length-1]='\0';
					 if (ALLOCATE(temp_string,char,length+13))
					 {
							strcpy(temp_string, "set dir \/d ");
							strcat(temp_string, drive_name);
							strcat(temp_string, "\\");
							temp_string[length+12]='\0';
							Execute_command_execute_string(file_open_data->execute_command,temp_string);
							DEALLOCATE(temp_string);
					 }
					 DEALLOCATE(drive_name);
					 if (length == lastlength)
					 {
							temp_name = const_cast<char *>(&filename[lastlength]);
					 }
				}
				if (lastlength>length)
				{
					 if (ALLOCATE(temp_directory_name,char,lastlength+1))
					 {
							strncpy(temp_directory_name,filename,lastlength);
							if (ALLOCATE(directory_name,char,lastlength-length+1))
							{
								 directory_name = &temp_directory_name[length-1];
								 directory_name[lastlength-length]='\0';
								 if (ALLOCATE(temp_string,char,lastlength-length+11))
								 {
										strcpy(temp_string, "set dir \'");
										strcat(temp_string, directory_name);
										strcat(temp_string, "\'");
										temp_string[lastlength-length+10]='\0';
										Execute_command_execute_string(file_open_data->execute_command,temp_string);
										temp_name = const_cast<char *>(&filename[lastlength]);
										DEALLOCATE(temp_string);
								 }
							}
							DEALLOCATE(temp_directory_name);
					 }
				}
				else if (lastlength==length)
				{
					temp_name = const_cast<char *>(&filename[lastlength]);
				}
				if (file_open_data->operation)
					(file_open_data->operation)((temp_name), file_open_data->arguments);
		 }
#else /*defined (WIN32_SYSTEM)*/
		 if (file_open_data->operation)
		 {
				(file_open_data->operation)((file_open_data->file_name)
							,file_open_data->arguments);
		 }
#endif /* !defined (WIN32_SYSTEM)*/
	}
#endif /* defined (WX_USER_INTERFACE) */
	LEAVE;
} /* open_file_and_write */

int register_file_cancel_callback(struct File_open_data *file_open_data,
	File_cancel_operation cancel_operation,void *cancel_arguments)
/*******************************************************************************
LAST MODIFIED : 6 July 1999

DESCRIPTION :
Register a callback that gets called when the file dialog is cancelled.
==============================================================================*/
{
	int return_code;

	ENTER(register_file_cancel_callback);
	return_code=0;
	if (file_open_data&&cancel_operation)
	{
		file_open_data->cancel_operation=cancel_operation;
		file_open_data->cancel_arguments=cancel_arguments;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"register_file_cancel_callback.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* register_file_cancel_callback */

#if defined (WX_USER_INTERFACE)
int filedir_compressing_process_wx_compress(const char *com_file_name, const char *exfile_name,
	 int exfile_return_code, const char *file_name, const char *temp_exfile)
/*******************************************************************************
LAST MODIFIED : 17 Aug 2007

DESCRIPTION :
Zip .com, .exnode, .exelem and .exdata files into a single zip file
==============================================================================*/
{
	 int return_code = 0;
	 char *zip_file_name;
	 if (!file_name)
	 {
			file_name = "temp";
	 }

	 int length = strlen(file_name);
	 if (ALLOCATE(zip_file_name, char, length+6))
	 {
			strcpy(zip_file_name, file_name);
			strcat(zip_file_name, ".zip");
			zip_file_name[length+5]='\0';
			wxFFileOutputStream out(wxString::FromAscii(zip_file_name));
			wxZipOutputStream zip(out);

			if (exfile_return_code)
			{
				 wxFFileInputStream data_in(wxString::FromAscii(temp_exfile), wxT("rb"));
				 zip.PutNextEntry((wxFileName(wxString::FromAscii(exfile_name))).GetFullName());
				 zip.Write(data_in);
			}
			wxFFileInputStream com_in(wxString::FromAscii(com_file_name),wxT("rb"));
			zip.PutNextEntry((wxFileName(wxString::FromAscii(com_file_name))).GetFullName());
			zip.Write(com_in);
			return_code = zip.Close();

			DEALLOCATE(zip_file_name);
	 }
	 return(return_code);
}
#endif /* defined (WX_USER_INTERFACE) */
