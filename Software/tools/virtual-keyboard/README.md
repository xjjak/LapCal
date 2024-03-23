# Description

This is a web server written in Flask.  It provides a virtual copy of one half of the Ferris Sweep keyboard.  All the key events are logged and sent back to the server.

This is meant to be used on a tablet to collect labels along with the data from gloves.

# Dependencies

This tool depends on:
- `python` (3.11)
- `flask`

The easiest way to acquire the dependencies is to use the `shell.nix` configuration with:
``` sh
nix-shell
```
  
# Usage

To run the web server run:
``` sh
python server.py
```
Alternatively, if you have nix installed, you can also run:
``` sh
./sever.py
```
This will actually automatically load all dependencies specified in the `shell.nix` file.

If you use this within your local network (recommended) Flask will tell you the IP address with which you can access the server.  The following table shows all available paths and their function.
| Path     | Function                                                                                                                           |
|----------|------------------------------------------------------------------------------------------------------------------------------------|
| `/`      | The virtual keyboard itself can be found at the root.  You can just start using the keyboard.                                      |
| `/clear` | Accessing this path clears the log maintained by the webserver (not the one maintained by the client).                             |
| `/logs`  | This path returns the current log of the server in currently *HTML format*.  To use this data you should save it as a `.txt` file. |
