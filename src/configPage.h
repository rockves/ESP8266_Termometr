const char page[] = R"=====(
<!DOCTYPE html>
<HTML>
	<HEAD>
		<TITLE>Configuration page</TITLE>
	</HEAD>
	<BODY>
		<CENTER>
			<form action="post" method = "POST">
				SSID:<br>
				<input type="text" name="SSID" value=""><br>
				PASSWORD:<br>
				<input type="text" name="PASSWORD" value=""><br><br>
				<input type="submit" value="Connect">
			</form> 
		</CENTER>
	</BODY>
</HTML>
)=====";