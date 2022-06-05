<?php
$config = file_get_contents("config.json"); 
$MCobj = json_decode($config,true);
echo $MCobj['year'];                 
echo $MCobj['month'];                         
echo $MCobj['day'];     
echo $MCobj['hour'];    
echo $MCobj['min'];    
echo $MCobj['sec'];    
echo $MCobj['span'];    
echo $MCobj['ml'];    
?>
