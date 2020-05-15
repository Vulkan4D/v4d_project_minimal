set -e

echo "Running unit tests DEBUG for Windows..."
ssh WINDOWS_PC "cd /v4d_build_minimal/debug/ && tests.exe"

echo "Running unit tests RELEASE for Windows..."
ssh WINDOWS_PC "cd /v4d_build_minimal/release/ && tests.exe"
