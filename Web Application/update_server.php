<?php

include_once "assets/class.php";

// Define the new 'wave' object:
$wave = new _main();
$wave->__init__($conn);

# Get the current date and time.
$date = date("Y_m_d_H_i_s");

# Create the image file name. 
$img_file = "PIPE_".$date;

// If Arduino Nicla Vision sends the collected 60GHz mmWave sensor data with the selected pipeline diagnostic class, save the received information to the entries MySQL database table.
if(isset($_GET["data"]) && isset($_GET["mmWave"]) && isset($_GET["class"])){
	if($wave->insert_new_data($date, $_GET["mmWave"], $_GET["class"])){
		echo "New Data Record Saved Successfully!";
	}else{
		echo "Database Error!";
	}
}

// If Arduino Nicla Vision sends the model detection results, save the received information to the detections MySQL database table.
if(isset($_GET["results"]) && isset($_GET["mmWave"]) && isset($_GET["class"])){
	if($wave->insert_new_results($date, $_GET["mmWave"], $img_file.".jpg", $_GET["class"])){
		echo "Detection Results Saved Successfully!";
	}else{
		echo "Database Error!";
	}
}

// If Arduino Nicla Vision transfers an image of a deformed pipe after running the neural network model, save the received raw image buffer (RGB565) as a TXT file to the detections folder.
if(!empty($_FILES["captured_image"]['name'])){
	// Image File:
	$captured_image_properties = array(
	    "name" => $_FILES["captured_image"]["name"],
	    "tmp_name" => $_FILES["captured_image"]["tmp_name"],
		"size" => $_FILES["captured_image"]["size"],
		"extension" => pathinfo($_FILES["captured_image"]["name"], PATHINFO_EXTENSION)
	);
	
    // Check whether the uploaded file extension is in the allowed file formats.
	$allowed_formats = array('jpg', 'png', 'bmp', 'txt');
	if(!in_array($captured_image_properties["extension"], $allowed_formats)){
		echo 'FILE => File Format Not Allowed!';
	}else{
		// Check whether the uploaded file size exceeds the 5 MB data limit.
		if($captured_image_properties["size"] > 5000000){
			echo "FILE => File size cannot exceed 5MB!";
		}else{
			// Save the uploaded file (image).
			move_uploaded_file($captured_image_properties["tmp_name"], "./detections/".$img_file.".".$captured_image_properties["extension"]);
			echo "FILE => Saved Successfully!";
		}
	}
	
	// Convert the recently saved RGB565 buffer (TXT file) to a JPG image file by executing the rgb565_converter.py file.
	$raw_convert = shell_exec('python "C:\Users\kutlu\New E\xampp\htdocs\pipeline_diagnostics_interface\detections\rgb565_converter.py"');
	print($raw_convert);

	// After generating the JPG file, remove the recently saved TXT file from the server.
	unlink("./detections/".$img_file.".txt");
}

// If requested, create a CSV file from the data records saved in the entries database table.
if(isset($_GET["create_CSV"])){
	// Create the data_records.csv file.
	$filename = $wave->create_CSV();
	// Download the generated CSV file automatically.
    header('Content-Description: File Transfer');
    header('Content-Type: application/octet-stream');
    header("Cache-Control: no-cache, must-revalidate");
    header('Content-Disposition: attachment; filename="'.basename($filename).'"');
    header('Content-Length: '.filesize($filename));
	header('Pragma: public');
    readfile($filename);
}

?>