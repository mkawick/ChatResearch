
<html>
<title>Playdek Games Reset Account Password</title>
	<script>
	<!--<br />
<b>Warning</b>:  mysqli::mysqli(): (HY000/2002): A connection attempt failed because the connected party did not properly respond after a period of time, or established connection failed because connected host has failed to respond.
 in <b>C:\xampp\htdocs\test_db\include\db.php</b> on line <b>11</b><br />
<br />
<b>Warning</b>:  mysqli::set_charset(): Couldn't fetch mysqli in <b>C:\xampp\htdocs\test_db\include\db.php</b> on line <b>12</b><br />
 -->
	function validateEmail(email) { 
	// http://stackoverflow.com/a/46181/11236
  
		var re = /^(([^<>()[\]\\.,;:\s@\"]+(\.[^<>()[\]\\.,;:\s@\"]+)*)|(\".+\"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;
		return re.test(email);
	}

	function validateForm() {
		var emailAddr = document.forms["myForm"]["email"].value;
		if( !emailAddr ) {
			alert( "In order to reset your password, you must provide an email address." );
			emailAddr.focus();
			return false;
		}
		if( validateEmail( emailAddr ) ) {
		}
		else{
			alert( "That appears to be an invalid email address. Please try again." );
			return false;
		}

		var pwd=document.forms["myForm"]["pwd"];
		var pwd2=document.forms["myForm"]["pwd2"];
		/*alert( pwd.value );
		alert( pwd2.value );
		alert( pwd.value != pwd2.value );*/
		if( pwd.value == "" ) {
			alert( "Please enter a password" );
			pwd.focus();
			return false;
		}
		if( pwd.value != pwd2.value ) { 
		   alert( "Your password and confirmation password do not match." );
		   pwd2.focus();
		   return false; 
		}
		return true;
	}
	</script>
<style>

p
	{
	margin-right:10px;

	margin-left:10px;

	font-size:12pt;
	font-family:"Geneva",}
.style4 {font-family: Geneva, Arial, Helvetica, sans-serif; font-size: 9px; }
.style6 {font-family: Geneva, Arial, Helvetica, sans-serif; font-weight: bold; }
.style7 {font-family: Geneva, Arial, Helvetica, sans-serif; font-size: 12px;}
.style9 {font-family: Geneva, Arial, Helvetica, sans-serif; font-size: 12px;}
.style10 {font-family: Geneva, Arial, Helvetica, sans-serif; font-size: 12px; }
.style12 {color: #242E38}
.style13 {font-family: Geneva, Arial, Helvetica, sans-serif}
</style>

</head>
<body lang=EN-US link=rgb(228,186,52) vlink=darkgrey alink=darkgrey>
<div id=div>
  <div>
    <div align=center>
      <table Table border=0 cellspacing=0 cellpadding=0 width=893 >



        <tr id=banner>
          <td colspan=2 class="style7" ><p align="left" >&nbsp;

          </p></td>
          <td bgcolor="#242E38" class="style7"><div align="center"><img src="http://accounts.playdekgames.com/images/playdek_email_banner_small_i.jpg" width="384" height="106" align="top"></div></td>
          <td class="style7">&nbsp;</td>
        </tr>
        <tr >
          <td colspan=2 class="style7" ><p><img src="spacer.gif" width="1" height="1" border=0>

          </p></td>
          <td bgcolor="#EEEEEE" class="style7">&nbsp;</td>
          <td class="style7">&nbsp;</td>
        </tr>
        <tr>
          <td colspan=2 class="style7"><p ><img src="spacer.gif" width="1" height="1" border=0>

          </p></td>
          <td width=768 bgcolor="#EEEEEE" class="style9" >
		  
			<div >
					<table width="95%" border="0" align="center" class="style9">
					  <tr>
							<td>
								You have requested to change your Playdek Account Password. Please enter the email address associated with your account and a new password. 
								A confirmation will be sent to the email account you provide. Your password will NOT be changed until you click the confirmation included in the email. 
								Please make a note of your new password. 			
								<br>
								</br>							</td>
					  </tr>
					</table>
			</div>
			<table width="340" border="0" align="center" class="style9" >

				<form name="myForm" action="http://accounts.playdekgames.com/test_db/reset_password_result.php" method="post" class="input_field" onSubmit="return validateForm();">
					<td align="right">
							<label for="username">Email Address: </label> 
							<input type="text" name="email" id="username" size ="30" > <br>
							<label for="pwd">New Password: </label>
							<input type="password" name="pwd" id="pwd"size ="30" > <br>
							<label for="pwd2">Confirm Password: </label>
							<input type="password" name="pwd2" id="pwd2"size ="30" > <br>
							&nbsp; 
							<div align="center">
							<input type="submit" value="Submit New Password" > 	<br></br>
							</div>					</td>
				</form>	
            </table>

						<div >
					<table width="95%" border="0" align="center" class="style9">
					  <tr>
							<td>
								If you experience any issues with  playing our games please check <a href="http://www.playdekgames.com/support.php">http://www.playdekgames.com/support.php</a> 
								or email <a href="mailto:support@playdekgames.com">support@playdekgames.com</a>.  
								Please include the following information: the device you are playing on, the OS version installed on the device, 
								whether you have the latest update to the game, and any specifics about the exact condition.					<p align="center" class="style9"><a href="tos.html" id="tos"> Click here to see our terms of service </a></p>		</td>
					  </tr>
					</table>
			</div>	
     
            </td>
          <td width=70 class="style7"> <p><img src="spacer.gif"  width="1" height="1" border=0>

          </p></td>
        </tr>
        <tr>
          <td colspan=2 class="style7" ><p ><img src="spacer.gif" width="1" height="1" border=0>

          </p></td>
          <td bgcolor="#EEEEEE" height="40"><hr size=1 width="100%" align=center></td>
          <td class="style7">&nbsp;</td>
        </tr>
 
        <tr style='height:22.5pt'>
          <td colspan=2 class="style7"><p><img src="spacer.gif" width="1" height="1" border=0>
          </p></td>
          <td bgcolor="#EEEEEE" class="style7" id = explanation> <p align="center" class="style10">
		  Other titles currently available from Playdek:</p>
            <p align="center"><span class="style10">
			<a href="https://itunes.apple.com/us/app/lords-of-waterdeep/id648019675?mt=8">Lords of Waterdeep</a> |
			<a href="https://itunes.apple.com/us/app/agricola/id561521557?mt=8">Agricola</a> | 
			<a href="https://itunes.apple.com/us/app/summoner-wars/id493752948?mt=8">Summoner Wars</a> | 
			<a href="https://itunes.apple.com/us/app/ascension-chronicle-godslayer/id441838733?mt=8">Ascension: Chronicle of the  Godslayer</a> | 
			<a href="https://itunes.apple.com/us/app/fluxx/id561319376?mt=8">Fluxx</a>  |
			<a href="https://itunes.apple.com/us/app/nightfall/id493828796?mt=8">Nightfall</a> 

				</br>
		    <a href="https://itunes.apple.com/us/app/tanto-cuore/id635555487?mt=8">Tanto Cuore</a> |
			<a href="https://itunes.apple.com/us/app/penny-arcade-game-gamers-vs./id538996749?ls=1&mt=8">Penny Arcade The Game: Gamers vs  Evil</a> |
			<a href="https://itunes.apple.com/us/app/food-fight-ios/id493347343?mt=8">Food  Fight iOS</a> |
			<a href="https://itunes.apple.com/us/app/cant-stop/id557688143?mt=8">Can't Stop</a></span></p>
			

          </td>
          <td class="style7" ></td>
        </tr>		
        <tr>
          <td colspan=2 style='height:22.5pt'><p><img src="spacer.gif" width="1" height="1" border=0>

          </p></td>
          <td bgcolor="#EEEEEE" class="style7">&nbsp;</td>
          <td class="style7" >&nbsp;</td>
        </tr>
        <tr >
          <td width=54 height="40" class="style7"><p><img src="spacer.gif"  width="1" height="1" border=0>

          </p></td>
          <td height="40" colspan=2 class="style7" id=social style='background:#242e38;background-position:initial initial;background-repeat:no-repeat'><div align=center>
            <table Table border=0 cellspacing=0 cellpadding=0 width=650 style='background:#242e38;background-position:initial initial;background-repeat:no-repeat'>
              <tr>
                <td width=400 style='background-position:initial initial;background-repeat:no-repeat'><p  align=right style='text-align:right'><span style='font-family:"Georgia","serif";color:white'><span class="style6">FOLLOW PLAYDEK:</span> &nbsp; &nbsp;

                </span></p></td>
                <td width=40 ><p ><span style='font-family:"Georgia","serif";color:white'><a href="https://www.facebook.com/Playdek" target="_blank"><b><span style='color:#242e38;text-decoration:none'><img src="http://accounts.playdekgames.com/images/icon_facebook.png"  width="33" height="32" border=0 ></span></b></a>

                </span></p></td>
                <td width=40 ><p ><span style='font-family:"Georgia","serif";color:white'><a href="https://twitter.com/playdek" target="_blank"><b><span style='color:#242e38;text-decoration:none'><img src="http://accounts.playdekgames.com/images/icon_twitter.png"  width="33" height="32" border=0 ></span></b></a>

                </span></p></td>
                <td width=200 ><p ><span style='font-family:"Georgia","serif";color:white'><a href="https://www.youtube.com/user/IncineratorStudios?feature=mhee" target="_blank"><b><span style='color:#242e38;text-decoration:none'><img src="http://accounts.playdekgames.com/images/icon_youtube.png"  width="33" height="32" border=0 ></span></b></a>

                </span></p></td>
              </tr>
            </table>
          </div></td>
          <td width=70 height="40" class="style7" ><p ><img src="spacer.gif"  width="1" height="1" border=0>

          </p></td>
        </tr>
        <tr>
          <td width=54 height = "32" class="style7"><p ><img src="spacer.gif"  width="1" height="1" border=0 >

          </p></td>
          <td colspan=2 class="style7" id=legal style='background:#EEEEEE '><div>
            <p  align=center class="style7" style='text-align:center'><span class="style4" style='color:#354751'>&copy;2010-2014 Playdek, All rights reserved. Playdek, and all associated logos and designs are trademarks or registered trademarks of Playdek. All other trademarks are the property of their respective owners. <a href="http://www.playdekgames.com/help.php?help=terms.php&amp;title=Terms of Use and Privacy Policy" >Terms of Use</a> | <a href="http://www.playdekgames.com/help.php?help=terms.php&amp;title=Terms of Use and Privacy Policy#Privacy" class="style12">Privacy Policy</a></span></p>
          </div></td>
          <td width=70 class="style7" ><p><img src="spacer.gif" width="1" height="1" border=0>

          </p></td>
        </tr>
<tr>
          <td width=54 height="10" class="style7"><p ><img src="spacer.gif" width="1" height="1" border=0>

          </p></td>
          <td colspan=2 class="style7" ><table width=250 border=0 align="center" cellpadding=0 cellspacing=0 id=social>
  
          </table></td>
          <td width=70 class="style7"><p ><img src="spacer.gif"  width="1" height="1" border=0 >

          </p></td>
        </tr>		
        <tr>
          <td width=54 class="style7" ><p ><img src="spacer.gif" width="1" height="1" border=0>

          </p></td>
          <td colspan=2 class="style7" ><div align="center">
				<img src="http://accounts.playdekgames.com/images/icon_gg_2011.png" width="72" height="72">
				<img src="http://accounts.playdekgames.com/images/icon_gg_2012.png" width="72" height="72">
				<img src="http://accounts.playdekgames.com/images/icon_gg_2013.png" width="72" height="72">
				<img src="http://accounts.playdekgames.com/images/icon_PGaward_gold.png" width="72" height="72">
				<img src="http://accounts.playdekgames.com/images/icon_PGaward_silver.png" width="72" height="72">
				<img src="http://accounts.playdekgames.com/images/icon_PGaward_bronze.png" width="72" height="72">
			  </div>
            <table width=250 border=0 align="center" cellpadding=0 cellspacing=0 id=social>
          </table></td>
          <td width=70 class="style7"><p ><img src="spacer.gif"  width="1" height="1" border=0 >

          </p></td>
        </tr>
        <tr>
          <td width=54 class="style7" ><p ><img src="spacer.gif" width="1" height="1" border=0 >

          </p></td>
          <td colspan=2 class="style7" ><div  align=center style='text-align:center'><span style='color:#354751'>
            <hr size=1 width="100%" align=center>
          </span></div>
              <p  align=center class="style4" style='text-align:center'><span style='color:#354751'>Playdek | Corporate Office | 2382 Faraday Ave.| Suite 130 | Carlsbad, CA 92008 | USA

            </span></p></td>
          <td width=70 class="style7" ><p ><img src="spacer.gif" name="_x0000_i1070" width="1" height="1" border=0 id="_x0000_i1070">

          </p></td>
        </tr>
      </table>
    </div>
  </div>

  

  </p>
</div>
<p class="style7">&nbsp;</p>
</body>
</html>