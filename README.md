This is a minimal version of the **v4d_project** repository

## Building a new minimal Vulkan4D project
(Only supported on Linux at the moment)

```bash
# from project parent directory
git clone --recursive git@github.com:Vulkan4D/v4d_project_minimal.git
cd v4d_project_minimal
tools/cleanbuild.sh
```
Unit tests will run after the build

You may want to make it your own git repository

```bash
# from project directory
tools/initNewGitRepository.sh
git remote add origin <Your Repository Url>
git push -u -f origin master
```

See V4D documentation in [the full v4d_project repository](https://github.com/Vulkan4D/v4d_project)
