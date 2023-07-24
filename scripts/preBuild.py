import os
import subprocess
import re

Import("env")
print("Pre build script")

# Reads the current git tag of the repo and returns the version number 
# elements
# In case there are changes since the last tag, the dirty flag is set
# In case the git tag does not match the x.y.z format, 0.0.0 is used as fallback
def git_version():
    def _minimal_ext_cmd(cmd):
        # construct minimal environment
        env = {}
        for k in ['SYSTEMROOT', 'PATH']:
            v = os.environ.get(k)
            if v is not None:
                env[k] = v
        # LANGUAGE is used on win32
        env['LANGUAGE'] = 'C'
        env['LANG'] = 'C'
        env['LC_ALL'] = 'C'
        out = subprocess.Popen(cmd, stdout = subprocess.PIPE, env=env, shell=True).communicate()[0]
        return out

    fw_maj = "0"
    fw_min = "0"
    fw_patch = "0"
    fw_suffix = ""

    try:
        out = _minimal_ext_cmd("git describe --tags")
        version_string = out.strip().decode('ascii')
        print(version_string)

        # Check if string matches the version format 0.0.0-...
        regex = r'\d{,3}[.]\d{1,3}[.]\d{1,3}'

        pattern = re.compile(regex)
        match = pattern.match(version_string)

        if match:
            fw_maj, fw_min, tail = version_string.split('.')
            tail = tail.split('-', 1)
            fw_patch = tail[0]
            if len(tail) > 1:
                # Maximum length of suffix: 16 characters
                fw_suffix = tail[1][:16]
    except OSError:
        pass

    return fw_maj, fw_min, fw_patch, fw_suffix

fw_maj, fw_min, fw_patch, fw_suffix = git_version()
print("GIT Version: " + fw_maj + "." + fw_min + "." + fw_patch + "-" + fw_suffix)

with open("src/ayab/version.h", "w") as text_file:
    text_file.write("constexpr uint8_t FW_VERSION_MAJ = {0}U;\n".format(fw_maj))
    text_file.write("constexpr uint8_t FW_VERSION_MIN = {0}U;\n".format(fw_min))
    text_file.write("constexpr uint8_t FW_VERSION_PATCH = {0}U;\n".format(fw_patch))
    text_file.write("constexpr char  FW_VERSION_SUFFIX[] = \"{0}\";\n".format(fw_suffix))
