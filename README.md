# Basic_HTTP_1.0_Server_C
Micro HTTP 1.0 Server in C for Linux

It is a tiny HTTP 1.0 Server, which does only supports:	
								
--[200] ok ( if file is found from GET  )	
																													
--[404] not found ( if file is not found or cant read from GET )

--[501] not implemented ( Anything else )

but he can do that parallel

it only supports 1 subdirectory, where ALL the html / htm 
files CAN be located, trying to open any other File will
fail if the path to file is given from socket
