<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>POST_SAMPLE</title>
</head>
<body>
<?php
    echo "送信完了<br>";
    echo $_POST["year"] ."/".$_POST["month"] ."/".$_POST["day"] ." ".$_POST["hour"]."時";
    echo "<br>";
    echo "<a href = ./index.php>back page</a>";
?>

<?php

  file_put_contents("config.json", '{'.'"year": '.$_POST["year"] .', "month": '.$_POST["month"] .', "day": '.$_POST["day"] .', "hour": '.$_POST["hour"] .', "min": 0, "sec": 0, "span": '.$_POST["span"] .', "ml": '.$_POST["ml"].'}');

echo '<a href="./index.php">前に戻る</a>';
header('Location: '.$_SERVER['HTTP_REFERER']);
exit();
?>

</body>
</html>
