<?php

// curl -X POST -d "BMP085_ERROR=0&DEVICE_VER=100&DHT22_ROSNY_BOD=8.5&BMP085_PRES_VAL=983&BMP085_TEMP_VAL=20.6&DHT22_HUM_VAL=45.1&DHT22_ERROR=0&BMP085_REL_TLAK=1015.8&DHT22_TEMP_VAL=20.9" http://domain.tld/test.php

$servername = "localhost";
$username = "meteostanice";
$password = "tajneHeslo123";
$dbname = "meteostanice";

if( !isset($_POST['DEVICE_VER']) ) {
  die('Nebyla poslana platna data');
}

$conn = mysql_connect($servername, $username, $password);

if(! $conn ) {
  die('Could not connect: ' . mysql_error());
}

mysql_select_db($dbname);

$date = date('Y-m-d H:i:s');
//mysql_query("INSERT INTO `table` (`dateposted`) VALUES ('$date')");


$DEVICE_VER=$_POST['DEVICE_VER'];
$DHT22_ERROR=$_POST['DHT22_ERROR'];
$BMP085_ERROR=$_POST['BMP085_ERROR'];
$DHT22_TEMP_VAL=$_POST['DHT22_TEMP_VAL'];
$DHT22_HUM_VAL=$_POST['DHT22_HUM_VAL'];
$DHT22_ROSNY_BOD=$_POST['DHT22_ROSNY_BOD'];
$BMP085_TEMP_VAL=$_POST['BMP085_TEMP_VAL'];
$BMP085_PRES_VAL=$_POST['BMP085_PRES_VAL'];
$BMP085_REL_TLAK=$_POST['BMP085_REL_TLAK'];

$sql = "INSERT INTO data".
"(DATETIME, DEVICE_VER, DHT22_ERROR, BMP085_ERROR, DHT22_TEMP_VAL, DHT22_HUM_VAL, DHT22_ROSNY_BOD, BMP085_TEMP_VAL, BMP085_PRES_VAL, BMP085_REL_TLAK)".
"VALUES ".
"('$date','$DEVICE_VER','$DHT22_ERROR','$BMP085_ERROR','$DHT22_TEMP_VAL','$DHT22_HUM_VAL','$DHT22_ROSNY_BOD','$BMP085_TEMP_VAL','$BMP085_PRES_VAL','$BMP085_REL_TLAK')";

$retval = mysql_query( $sql, $conn );
if(! $retval ) {
  die('Could not enter data: ' . mysql_error());
}
echo "Entered data successfully\n";
mysql_close($conn);

?>


