<html>
<head>
<title>
水やり2
</title>
<meta charset="UTF-8">
<link rel="stylesheet" href="style.css">
</head>
<body>


<p>水やり設定</p>
<table>
	<tbody>
		<tr>
			<td class="item">開始タイミング</td>
			<td class="value">
			    <form method="post" action="setConfig.php">
                <select name="year" >
                <?php
                    $config = file_get_contents("config.json"); 
                    $obj = json_decode($config,true);
                    $value = $obj["year"];
                    echo '<option value="'.$value.'" selected>'.$value.'</option>';
                ?>
                <option value="2022">2022</option>
                <option value="2023">2023</option>
                <option value="2024">2024</option>
                <option value="2025">2025</option>
                <option value="2026">2026</option>
                <option value="2027">2027</option>
                <option value="2028">2028</option>
                <option value="2029">2029</option>
                <option value="2030">2030</option>
                </select>　年

                <select name="month">
                <?php
                    $config = file_get_contents("config.json"); 
                    $obj = json_decode($config,true);
                    $value = $obj["month"];
                    echo '<option value="'.$value.'" selected>'.$value.'</option>';
                ?>
                <option value="1">1</option>
                <option value="2">2</option>
                <option value="3">3</option>
                <option value="4">4</option>
                <option value="5">5</option>
                <option value="6">6</option>
                <option value="7">7</option>
                <option value="8">8</option>
                <option value="9">9</option>
                <option value="10">10</option>
                <option value="11">11</option>
                <option value="12">12</option>
                </select>　月

                <select name="day">
                <?php
                    $config = file_get_contents("config.json"); 
                    $obj = json_decode($config,true);
                    $value = $obj["day"];
                    echo '<option value="'.$value.'" selected>'.$value.'</option>';
                ?>
                <option value="1">1</option>
                <option value="2">2</option>
                <option value="3">3</option>
                <option value="4">4</option>
                <option value="5">5</option>
                <option value="6">6</option>
                <option value="7">7</option>
                <option value="8">8</option>
                <option value="9">9</option>
                <option value="10">10</option>
                <option value="11">11</option>
                <option value="12">12</option>
                <option value="13">13</option>
                <option value="14">14</option>
                <option value="15">15</option>
                <option value="16">16</option>
                <option value="17">17</option>
                <option value="18">18</option>
                <option value="19">19</option>
                <option value="20">20</option>
                <option value="21">21</option>
                <option value="22">22</option>
                <option value="23">23</option>
                <option value="24">24</option>
                <option value="25">25</option>
                <option value="26">26</option>
                <option value="27">27</option>
                <option value="28">28</option>
                <option value="29">29</option>
                <option value="30">30</option>
                <option value="31">31</option>
                </select>　日

                <select name="hour">
                <?php
                    $config = file_get_contents("config.json"); 
                    $obj = json_decode($config,true);
                    $value = $obj["hour"];
                    echo '<option value="'.$value.'" selected>'.$value.'</option>';
                ?>
                <option value="0">0</option>
                <option value="1">1</option>
                <option value="2">2</option>
                <option value="3">3</option>
                <option value="4">4</option>
                <option value="5">5</option>
                <option value="6">6</option>
                <option value="7">7</option>
                <option value="8">8</option>
                <option value="9">9</option>
                <option value="10">10</option>
                <option value="11">11</option>
                <option value="12">12</option>
                <option value="13">13</option>
                <option value="14">14</option>
                <option value="15">15</option>
                <option value="16">16</option>
                <option value="17">17</option>
                <option value="18">18</option>
                <option value="19">19</option>
                <option value="20">20</option>
                <option value="21">21</option>
                <option value="22">22</option>
                <option value="23">23</option>
                </select>　時

			</td>
		</tr>
		<tr>
			<td class="item">水やりの間隔</td>
			<td class="value">
			    <select name="span">
                <?php
                    $config = file_get_contents("config.json"); 
                    $obj = json_decode($config,true);
                    $value = $obj["span"];
                    echo '<option value="'.$value.'" selected>'.$value.'</option>';
                ?>
                <option value="1">1</option>
                <option value="2">2</option>
                <option value="3">3</option>
                <option value="4">4</option>
                <option value="5">5</option>
                <option value="6">6</option>
                <option value="7">7</option>
                <option value="8">8</option>
                <option value="9">9</option>
                <option value="10">10</option>
                <option value="11">11</option>
                <option value="12">12</option>
                <option value="24">24</option>
                </select>　時間おき
			</td>
		</tr>
		<tr>
			<td class="item">水量</td>
			<td class="value">
			
			    <select name="ml">
                <?php
                    $config = file_get_contents("config.json"); 
                    $obj = json_decode($config,true);
                    $value = $obj["ml"];
                    echo '<option value="'.$value.'" selected>'.$value.'</option>';
                ?>
                <option value="30">30</option>
                <option value="50">50</option>
                <option value="100">100</option>
                <option value="200">200</option>
                </select>　ml
			
			</td>
		</tr>
	</tbody>
</table>
<div><input class="submit" type="submit" value="送信"></div>
</form>
<br>





<p>ステータス</p>
<table>
	<tbody>
		<tr>
			<td class="item2">
			    設定<br>
			</td>
			<td class="value2">
                <?php
                    $config = file_get_contents("config.json"); 
                    $obj = json_decode($config,true);
                    echo $obj['year'].'/'.$obj['month'].'/'.$obj['day'].' '.$obj['hour'].':00'.'から'.$obj['span'].'時間ごとに'.$obj['ml'].'[ml]';
                ?>
			</td>
		</tr>
		<tr>
			<td class="item2">
			    死活情報<br>
			    （最後に通信を確認した日時）<br>
			</td>
			<td class="value2">
			    <?php
                $file = fopen("log/alive.txt", "r"); 
                if($file){
                  while ($line = fgets($file)) {
                    echo $line;
                    break;
                  }
                }
                fclose($file);
                ?>
			</td>
		</tr>
		<tr>
			<td class="item2">
			    水やり情報<br>
			    （水やりを実施した日時）<br>
			</td>
			<td class="value2">
			    <?php
                $file = fopen("log/water.txt", "r"); 
                if($file){
                  while ($line = fgets($file)) {
                    echo $line;
                    break;
                  }
                }
                fclose($file);
                ?>
			</td>
		</tr>
	</tbody>
</table>


</body>
</html>
