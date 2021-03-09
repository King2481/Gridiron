# Gridiron
 
## What is this?

This is an Arena-Style FPS that I built in my freetime. It includes 6 weapons (Shotgun, Supper Shotty, Sniper Rifle, Rocket Launcher, Plasma Cannon, and a Chaingun). Also features things such as fast movement, dashing, and rocket jumping. Supports two gamemodes: Deathmatch and Team Deathmatch.

## Okay, you have coded like three of these now. What is so fundementally different about this one?

Good question, compared to the last [prototype](https://github.com/King2481/Infinity) that I made, this one is much better polished and has features that made developing _much_ eaiser instead of having to try to bodge a mechanic together. Things such as:

1. A time limit so that way games don't go on forever.
2. Things such as a team chat for when you are playing TDM
3. Better visual representation for when you have Quad damage.
4. Better weapon behavior handling support.
5. Porting Shootergame's Highrise map so there is an actual map to play on instead of a test enviornment.
6. Ability System support
7. Third person sound support (walking, remote firing, etc.)

I also decided to cut a few features such as player skins as while they were nice, 

## Abilities?
I decided to take a look into using UE4's Gameplay Ability System for handling things such as Weapon Behavior as I found that the way that the previous game was coded, making the Sniper Rifle zoom in while wasn't difficult, the foundation made it to where Firearms could only ever support very specific behavior. 

With this project, I coded the Ability System into the weapons themselves and treated things such as firing and aiming as "behaviors". This allows me to create "alt fire" abilities much more efficiently and far less error prone than how the old system did it (which was handled as a Blueprint Event).

And I decided to not just end it with the weapons, I decided to extend it to the characters as well. Now characters can dash accross the screen briefly. And throw grenades too!

## Post-mortem
While I am happy with how this project ended (as I can finally say "look, it's a game!"). That's not to say I wish there were some things I could have implemented such as a a better way to show that a certain indevidual has the Quad Damage active. But overall, I'm happy I can end it and start up a new project (it won't be an Arena-FPS I swear!)
