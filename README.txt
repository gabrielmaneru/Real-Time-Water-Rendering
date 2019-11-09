Name: Gabriel Mañeru
Login: gabriel.m

Controls:
	-Shift+Leftclick for picking objects
	-Rightclick to enable movement
	-WASDQE for displacement (In Target & Freemode)
	-Mouse for rotating (In Target & Freemode)
	-Shift for accelerating movement
	-Ctrl+R & Ctrl+Q working


Options:
	-Curves:
		-Render Curves (Curve in green, points in blue, look-up table in red)
		-Change Epsilon for the adaptive forward differencing
		-Change Minimum forced subdivisions at adaptive forward differencing
		-Edit curve parameters:
			-Full Editable Easing bezier curve:
				-Break Tangents
				-Move Points/Tangents
				-Double left click for inserting new points
				-Right click for removing point
			-Look-up table for adaptive forward differencing
			-Center of interest target (if nothing is selected Frenet frame will be use)
			-In Frenet frame, second derivative was use to incline a bit the object to simulate fast centripetal motion.(However couldn't find a way to really showcase it)
	
Scene:
	-Reloading Scene
	-Create Objects
	
	-Lights Global Options -> Change Ambient for more clarity when testing the scene


Object Pick:
	-Edit Objects/Lights

Important files:
	-curve.cpp: evaluate_derivatives(), do_adaptive_forward_differencing(), distance_to_time(), evaluate_for_x()
	-scene_object.cpp: update()