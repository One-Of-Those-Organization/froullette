# Fate Roullete
Please read this file first when you decide to play this game for the first time (you 100% will be lost if you not!).

## Desc
This game inspired and copied (not all) from _Buckshot Roullete_. This game have 2 components which is `server` and `client`. You need to have the `server` running to play this game, you can self-host or use your friend computer.

## Running
### PC-Version
Launch the _.exe_ or the executable of the client on the correct dir (it should be in 1 dir with the `assets` dir)

### Web-Version
You can host it yourself or use this hosted git site: [github.io](https://one-of-those-organization.github.io/froullette-static-web/).

You dont need to do anything about the assets and stuff on this version.

### Mobile-Version (android)
Just launch the apps and you are done.

## Control inside the game (client)
* Left-click/tap the items to use them.
* Left-click and hold / hold the needle to move them around.
* Right-click/doubletap the needle to use them.

## More Info
* The revealer(the paper item) will reveal where is one of the `live` needles if there is no live or you are unlucky it will not reveal anything and your item gone.
* The boostes (the pill item) will boost the damage you will get from `live` and heal you by 1 heart if its empty.


## Server (SKIP if you dont want to self-host)
Server is needed for this game to func, we will not give the server binary you need to build it yourself.

__How to build the server:__

* Create a new dir with name of your chooice, `mkdir build`.
* Tell cmake to configure and build it in your dir, `cmake --build build --target froullete-server`
* Now you just need to execute the server that located in `./build/src/Server/froullete-server`
* The server support binding to other port and ip with the `-ip <ip>` and `-port <port>` if not set will use default of `0.0.0.0:8000`

--===#################===--
-==- GOOD LUCK PLAYING -==-
--===#################===--
