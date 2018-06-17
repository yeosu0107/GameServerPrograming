myid = 99999;
dist = 4;

function set_myid(x)
	myid = x;
end

function set_dist(x)
	dist = x;
end

function abs( len )
	if(len < dist) then
		if(len > -dist) then
			return true;
		end
	end
	return false;
end


function event_player_move(player)
	player_x = API_get_x(player);
	player_y = API_get_y(player);
	
	my_x = API_get_x(myid);
	my_y = API_get_y(myid);

	dir_x = player_x - my_x;
	dir_y = player_y - my_y;

	if(abs(dir_x)) then
		if(abs(dir_y)) then
			
			API_send_msg(player, myid, "HELLO!")
			API_npc_move(player, myid);
		end
	end
end