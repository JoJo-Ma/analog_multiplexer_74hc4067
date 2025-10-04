
# AppBlocks Zephyr Application

The project is set up to be opened from Visual Studio Code, containing relevant building and flashing scripts.

## Environment Variables

```
ZEPHYR_SDK_INSTALL_DIR
```
The path to the zephyr SDK installation directory

```
ZEPHYR_BASE
```
The path to the zephyr main project
```

OPENOCD_PATH
```
path to openocd (or openocd-esp32)

## Windows Configuration
```
Microsoft's Python extension must be installed in Visual Studio Code and the Python interepreter must be set to the "defaultInterpreterPath" defined in the .vscode/settings.json file. To do so, first uncomment the line in .vscode/settings.json file then open the command palette (Ctrl+Shift+P) and search for "Python: Select Interpreter". Select "Use python from 'python.defaultInterepreterPath' setting".
```


set OPENOCD_PATH or OPENOCD_ESP_PATH
            
