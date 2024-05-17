# File Transfer Socket programming
Modified version of the File Transfer Protocol (FTP) server program serve more than one user at a time to upload and download files to and from a specified directory 

# Usage:

Run the `server` application with the following command line arguments:


```
server -d <directory> -p <port> -u <password_file>
```

-d: Specify the `running directory` where files will be `accessed`, `modified`, or `erased.`
    
-p: Define the server `port number`.
   
-u: Provide the password file delimiter-separated format (using ":").

# Example:

```
$ server -d /home/student/files -p xxxx -u
```
File server listening on localhost port `xxxx`

**Cuation:** The server runs in an infinite loop, waiting for incoming connections until manually stopped

# Authentication:

Users must authenticate using their `username` and `password` from the provided password file. The format of the password file is as follows:

```
username1:password1
```
```
username2:password2
```
If the `username` and `password` match, the server responds with:

```
200 User <username1> granted to access
```
# Protocol
This version of FTP will be slight variation of the FTP protocol implemented over a TCP
connection which is guaranteed to be reliable
   
# Commands:
```
USER <Username> <PASS>
```
Server should handle simple `user` and `password` authentication from password file.

```
USER test test100
```
**200 User Username1 granted to access.**

```
USER nobody xxx
```
**400 User not found. Please try with another user.** 

If user `name` and `password` match server should give `“200”` response and allow following
commands otherwise should give Error code like `“400 User not found”`. If user not found
client only should use `command USER` or `QUIT`. 

```
LIST
```
If `client` gives this command in this case `server` should list `filenames` and `file sizes` to client. 
And after files listed list should terminated with `“.”`

```
GET <filename>
```
If client gives this command in this case `server` should read file content of filename and send 
content to client side. Also, after file content dumped it should terminate with `“\r\n.\r\n”`. If 
file is not found server should give `404 Error`

```
PUT <filename>
```
If `client` gives this command in this case `server` should read file content from `client` and save to `server` side.

```
PUT test
```
**This is text message that put as example**

**.**

**200 44 Byte test file retrieved by server and was saved.**

```
PUT test
```
**400 File cannot save on server side**

```
DEL <filename>
```
If `client` gives this command in this case server should `delete file` from `server side`. If its succeeded server should give `200 messages otherwise should give 404`

```
QUIT
```
If `client` gives this command in this case `server` should give goodbye message and terminates 
the client connection
