# SteamFps

Just a sample project walking through the setup of Steam services.  
This sample is based on the UE4 FPS C++ sample, starting with 4.13.1.

Note: This project uses git-lfs to store binary assets.


References for steam integration:
* Steam integration in Blueprint projects:  https://www.youtube.com/watch?v=ABWgSq5A9ak

Basic client connection flow:
* https://docs.unrealengine.com/latest/INT/Gameplay/Networking/Server/index.html

The major steps are:

* Client sends a connect request.
* If the server accepts, it will send the current map.
* The server will wait for the client to load this map.
* Once loaded, the server will then locally call AGameModeBase::PreLogin.
This will give the GameMode a chance to reject the connection
* If accepted, the server will then call AGameModeBase::Login
The role of this function is to create a PlayerController, which will then be replicated to the newly connected client. Once received, this PlayerController will replace the clients temporary PlayerController that was used as a placeholder during the connection process.
Note that APlayerController::BeginPlay will be called here. It should be noted that it is NOT yet safe to call RPC functions on this actor. You should wait until AGameModeBase::PostLogin is called.
Assuming everything went well, AGameModeBase::PostLogin is called.
* At this point, it is safe for the server to start calling RPC functions on this PlayerController.


Reference links along the way.

* https://answers.unrealengine.com/questions/2803/using-addtoroot-causes-crash-in-multiplayer.html
