Features that are desired, but for whatever reason can't be implemented without dependency upgrades or significant refactoring. Some of these are a bit far-fetched :).

Requirements are formatted as task lists so that TODO trackers catch them.
# WebGPU Builds
- [ ] Get it done
First big project, native web builds. A lot of work engine-wide to do this. [[VNWebGpu]] is still looking quite barren.
# Reflection for Handle Providers
- [ ] Upgrade to C++ 26
Affects [[VNAssets]], [[VNVulkan]], [[VNWebGpu]], and [[VNEngine]]. Using reflection instead of macros to generate declarations in native handles could tidy things up.
# `cstring_view` Instead of `const char*`
- [ ] Polyfill, since we're not getting this officially untill C++ 29.
Would give a nice memory safety boost project-wide.
# Continuous Integration
- [ ] Set up a Jenkins server
CI via Jenkins would be nice, but I don't really need it for a solo project. Use Jenkins over GitHub/Lab CI as I'd like to have incremental builds.
# Rigid CPP
- [ ] Use rigid where it make sense
Having read [the manifesto](https://github.com/I-A-S/Rigid-Cpp/blob/main/rigid-cpp.md), this seems like something that would be nice to have, at least in part, especially around exceptions which I don't have any recovery method from currently and shared pointers where STL has the overhead of weak pointers.
