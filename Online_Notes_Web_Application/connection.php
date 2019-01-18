<?php

$servername = "localhost";
$username = "yongpfvj_yhl";
$password = "i&VY8~c88s%$";
// $username = "yongpfvj";
// $password = "Liyongheng1001!";

$dbname = "yongpfvj_onlinenotes";

$link = mysqli_connect($servername, $username, $password, $dbname);

if(mysqli_connect_error()){
    die('ERROR: Unable to connect:' . mysqli_connect_error()); 
    echo "<script>window.alert('Hi!')</script>";
}

// $email = 'yonhenli@gmail.com';
// $subject = 'Confirm your Registration';
// $message = 'hello world!';


// if(mail($email, $subject, $message)) {
//        echo "<div class='alert alert-success'>Thank for your registring! A confirmation email has been sent to $email. 
//        Please click on the activation link to activate your account. </div>";
// }
// else {
//     echo "<div class='alert alert-success'> Failed to send the email. </div>";
// }

// echo "<h1> connected successful... </h1>";

// $sql = "CREATE TABLE tasks(ID INT(4) NOT NULL PRIMARY KEY AUTO_INCREMENT, firstname CHAR(20) NOT NULL, lastname CHAR(20) NOT NULL, email VARCHAR(30), password VARCHAR(40))";

// if(mysqli_query($link, $sql)){
//     echo "<p>Table 'task' created successfuly!</p>";   
// }else{
//     echo "ERROR: Unable to execute $sql" . mysql_error($link);   
// }

// $sql = "INSERT INTO tasks (firstname, lastname, email, password) VALUES ('Mark', 'Zuckerberg', 'M.zuckerberg@gmail.com', 'password')";
// if(mysqli_query($link, $sql)){
//     echo "<p>New row added successfully!</p>";  
// }else{
//     echo "ERROR: Unable to execute $sql" . mysql_error($link);   
// }

// $firstname = 'Sam';

// $sql = "SELECT 'firstname', 'lastname', 'email', 'password' FROM users WHERE 'firstname' = 'Mark'";
// $sql = "SELECT * FROM tasks WHERE firstname = '$firstname'";
// $sql = "SELECT firstname, lastname, email, password FROM tasks";

// $sql = "SELECT * FROM tasks";
// $result = mysqli_query($link, $sql);

// while ($row = mysqli_fetch_assoc($result)) {
//     echo "<tr>";
//     echo "<td>" . $row['firstname'] . "</td>";
//     echo "<td>" . $row['lastname'] . "</td>";
//     echo "<td>" . $row['email'] . "</td>";
//     echo "<td>" . $row['password'] . "</td>";
//     echo "</tr>";
// }

?>