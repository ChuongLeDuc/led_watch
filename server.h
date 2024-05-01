const char index_page[] PROGMEM = R"=====(

<!DOCTYPE html>
<html>
    <head>
        <title>SET UP CONNECTION</title>
        <style>
            body{
                text-align: center;
                background: #4fb39c2b;
            }

            table{
                margin:20px auto;
                text-align: left;
            }
            input{
                border: 1px solid green;
                border-radius: 3px;
                height: 25px;
            }
            select{
                border: 1px solid green;
                border-radius: 3px;
                height: 25px;
            }

            p{
                color: red;
            }

            .change_div{
                color:#7979bb;
            }

            .change_div:hover{
                cursor: pointer;
            }

            .button{
                background: #78dd6c;
                height: 50px;
                width: 80px;
                color: #005089;
                font-size: 15px;
                font-family: 'Times New Roman', Times, serif;
                border-radius: 5px;
                border: none;
            }
            #server_access{
                display: none;
            }
            #home_access{
                display: block;
            }
        </style>
    </head>

    <body>
        <div id="server_access">
            <p>SET UP DEVICE CONNECTION</p>
            <table>
                <tr>
                    <td>NEW DEVICE'S NAME</td>
                    <td><input type="text" maxlength="32" id="new_device_name" required></td>
                 </tr>

                <tr>
                    <td>OLD DEVICE'S PASSWORD</td>
                    <td><input type="password" minlength="8" maxlength="32" id="old_device_pass" required></td>
                   
                </tr>

                <tr>
                    <td>NEW DEVICE'S PASSWORD</td>
                    <td><input type="password" minlength="8" maxlength="32" id="new_device_pass" required></td>
                </tr>

                <tr>
                    <td>CONFIRM NEW PASSWORD</td>
                    <td><input type="password" minlength="8" maxlength="32" id="confirm_new_device_pass" required></td>
                </tr>
            </table>
            <input type="submit" value="UPDATE" onclick=update_device_wifi() class="button">
            <br><br>
            <span onclick=hide_server_div() class="change_div"><u>Change home wifi connection</u></span>
        </div>

        <div id="home_access">
            <p>SET UP HOME CONNECTION</p>
            <table>
                <tr>
                    <td>DEVICE'S PASSWORD</td>
                    <td><input type="password" minlength="8" maxlength="32" id="device_pass" required></td>
                </tr>

                <tr>
                    <td>HOME WIFI</td>
                    <td>
                        <select id="home_wifi">
                            <option>Choose your WiFi</option>
                        </select>
                    </td>
                </tr>

                <tr>
                    <td>HOME WIFI'S PASSWORD</td>
                    <td><input type="password" minlength="8" maxlength="32" id="home_pass" required></td>
                </tr>

                <tr>
                    <td>TIME ZONE</td>
                    <td>
                        <select id="time_zone">
                        </select>
                    </td>
                </tr>

            </table>
            <input type="submit" value="SAVE" onclick=update_home_wifi() class="button">
            <br><br>
            <span onclick=hide_home_div() class="change_div"><u>Change device wifi connection</u></span>
        </div>

        <div id="notification">
            <p id="update_status"></p>
        </div>

        <script>

            get_local_ssid();
            get_time_zone();

            function get_time_zone(){
                var option_html;
                for(let i=-12;i<=14;i++){
                    option_html+="<option>"+i+"</option>";
                }
                document.getElementById("time_zone").innerHTML=option_html;
            }

            function hide_server_div(){
                document.getElementById("server_access").style.display="none"
                document.getElementById('home_access').style.display="block";
                document.getElementById("update_status").innerHTML="";
                get_local_ssid();

            }

            function hide_home_div(){
                document.getElementById("server_access").style.display="block"
                document.getElementById('home_access').style.display="none";
                document.getElementById("update_status").innerHTML="";

            }

            function update_device_wifi(){

                var device_name=document.getElementById("new_device_name");
                var old_pass   =document.getElementById("old_device_pass");
                var new_pass   =document.getElementById("new_device_pass");
                var confirm_new_pass =document.getElementById("confirm_new_device_pass");

                var notification =document.getElementById("update_status");

                if(check_null_value(device_name)==false && check_null_value(old_pass)==false && check_null_value(new_pass)==false && check_null_value(confirm_new_pass)==false){
                    if(new_pass.value!=confirm_new_pass.value){
                        notification.innerHTML="Check your new password. Confirm password's not the same as new password!"
                    }else{
                        var xhttp=new XMLHttpRequest;
                        xhttp.open("GET","/device_wifi?device_name="+device_name.value+"&old_pass="+old_pass.value+"&new_pass="+new_pass.value,true);
                        console.log("/device_wifi?device_name="+device_name.value+"&old_pass="+old_pass.value+"&new_pass="+new_pass.value);
                        xhttp.onreadystatechange=function(){
                            if (this.readyState == 4 && this.status == 200) {
                                var respone=xhttp.responseText;
                                notification.innerHTML=respone;
                            }
                        }
                        xhttp.send();
                    }
                }else{
                    get_red_border(device_name);
                    get_red_border(old_pass);
                    get_red_border(new_pass);
                    get_red_border(confirm_new_pass);

                    notification.innerHTML="Check your Input!";

                } 
            }

            function update_home_wifi(){
                var device_pass=document.getElementById("device_pass");
                var home_wifi=document.getElementById("home_wifi");
                var home_pass=document.getElementById("home_pass");
                var time_zone=document.getElementById("time_zone");

                var notification =document.getElementById("update_status");

                if(check_null_value(device_pass)==false && check_null_value(home_wifi)==false && check_null_value(home_pass)==false){
                    if(home_wifi.value=="Choose your WiFi"){
                        notification.innerHTML="Select WiFi name!"
                    }else{
                        var xhttp=new XMLHttpRequest;
                        xhttp.open("GET","/local_wifi?device_pass="+device_pass.value+"&home_pass="+home_pass.value+"&home_ssid="+home_wifi.value+"&time_zone="+time_zone.value,true);
                        xhttp.onreadystatechange=function(){
                            if (this.readyState == 4 && this.status == 200) {
                                var respone=xhttp.responseText;
                                notification.innerHTML=respone;
                            }
                        }
                        xhttp.send();
                    }
                }else{
                    get_red_border(device_pass);
                    get_red_border(home_wifi);
                    get_red_border(home_pass);
                    notification.innerHTML="Check your Input!";

                } 
            }

            function get_local_ssid(){
                var xhttp=new XMLHttpRequest;
                xhttp.open("GET","/get_local_ssid",true);
                xhttp.onreadystatechange=function(){
                if (this.readyState == 4 && this.status == 200) {
                    var respone=xhttp.responseText;
                    document.getElementById("home_wifi").innerHTML=respone;
                }
                }
                xhttp.send();
            }


            function check_null_value(id_name){
                if(id_name.value==""){
                    return true;
                    document.getElementById("old_device_pass").style.border ="1px solid red";
                }else{
                    return false;
                }
            }

            function get_red_border(id_name){
                if(check_null_value(id_name)==true){
                    id_name.style.border="1px solid red";
                }
            }

        </script>


    </body>
</html>

)=====";
