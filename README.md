#### WRAPPER FOR X11 AND SDL3 (gpu support), IMGUI
- so its easier to use lol
- the wrapper is able to use X11, SDL3 (has GPU support, and imgui support) as a backend
- Example repo using the wrapper:
- https://github.com/FelixJaschul/wrapperTest.git

#### TL:DR
- code has to be ran in c++ (if using imgui) because there is no c binding for imgui rn
- X11 obv only runs on LINUX (tested) or WSL (tested) and on MACOS (my setup) if XQuartz is installed (because X11 ??)
- SDL3 runs on all platforms bla bla bla