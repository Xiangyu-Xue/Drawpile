Building AppImages with docker
------------------------------

First, create the build environment image:

    docker build -f Dockerfile -t drawpile .

Next, run the build scripts to build either the client and/or the server:

    mkdir out
    docker run -ti -v "$(pwd)/out:/out" drawpile bash /Build-client
    docker run -ti -v "$(pwd)/out:/out" drawpile bash /Build-server

