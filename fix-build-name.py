import shutil, os
Import("env")

env.Replace(PROGNAME = "ayab_monolithic_uno")
shutil.copy(os.getcwd()+"/.pio/build/uno/ayab_monolithic_uno.hex", 
    "/Applications/AYAB.app/Contents/Resources/ayab/firmware/ayab_monolithic_uno.hex")