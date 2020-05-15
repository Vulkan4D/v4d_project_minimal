cd "`dirname $0`/../.."
PROJECT_DIR=`pwd`
set -e

# Kill potentially running process on remote windows pc
ssh WINDOWS_PC "START /wait taskkill /f /im tests.exe"
ssh WINDOWS_PC "START /wait taskkill /f /im demo.exe"

# Delete build on remote windows pc
ssh WINDOWS_PC "for /d %i in (\v4d_build_minimal\debug\*) do @rmdir /s /q \"%i\" > NUL"
ssh WINDOWS_PC "del /q /s \v4d_build_minimal\debug\* > NUL"
ssh WINDOWS_PC "for /d %i in (\v4d_build_minimal\release\*) do @rmdir /s /q \"%i\" > NUL"
ssh WINDOWS_PC "del /q /s \v4d_build_minimal\release\* > NUL"


# Copy global DLLs to Remote Windows PC
scp -rq crosscompile/windows/dll/*.dll WINDOWS_PC:/v4d_build_minimal/debug/
scp -rq crosscompile/windows/dll/*.dll WINDOWS_PC:/v4d_build_minimal/release/

