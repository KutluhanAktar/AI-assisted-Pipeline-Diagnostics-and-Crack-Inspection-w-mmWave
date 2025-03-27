<?php

// Define the _main class and its functions:
class _main {
	public $conn;
	
	public function __init__($conn){
		$this->conn = $conn;
	}
	
    // Database -> Insert 60GHz mmWave Sensor Data:
	public function insert_new_data($date, $mmWave, $class){
		$sql_insert = "INSERT INTO `entries`(`date`, `mmwave`, `class`) 
		               VALUES ('$date', '$mmWave', '$class');"
			          ;
		if(mysqli_query($this->conn, $sql_insert)){ return true; } else{ return false; }
	}
	
    // Database -> Insert Model Detection Results:
	public function insert_new_results($date, $mmWave, $img, $class){
		$sql_insert = "INSERT INTO `detections`(`date`, `mmwave`, `img`, `class`) 
		               VALUES ('$date', '$mmWave', '$img', '$class');"
			          ;
		if(mysqli_query($this->conn, $sql_insert)){ return true; } else{ return false; }
	}
	
	// Retrieve all data records from the entries database table, transmitted by Arduino Nicla Vision.
	public function get_data_records(){
		$date=[]; $mmWave=[]; $class=[];
		$sql_data = "SELECT * FROM `entries` ORDER BY `id` DESC";
		$result = mysqli_query($this->conn, $sql_data);
		$check = mysqli_num_rows($result);
		if($check > 0){
			while($row = mysqli_fetch_assoc($result)){
				array_push($date, $row["date"]);
				array_push($mmWave, $row["mmwave"]);
				array_push($class, $row["class"]);
			}
			return array($date, $mmWave, $class);
		}else{
			return array(["Not Found!"], ["Not Found!"], ["Not Found!"]);
		}
	}
	
	// Retrieve all model detection results and the assigned detection image names from the detections database table, transferred by Arduino Nicla Vision.
	public function get_model_results(){
		$date=[]; $mmWave=[]; $class=[]; $img=[];
		$sql_data = "SELECT * FROM `detections` ORDER BY `id` DESC";
		$result = mysqli_query($this->conn, $sql_data);
		$check = mysqli_num_rows($result);
		if($check > 0){
			while($row = mysqli_fetch_assoc($result)){
				array_push($date, $row["date"]);
				array_push($mmWave, $row["mmwave"]);
				array_push($class, $row["class"]);
				array_push($img, $row["img"]);
			}
			return array($date, $mmWave, $class, $img);
		}else{
			return array(["Not Found!"], ["Not Found!"], ["Not Found!"], ["icon.png"]);
		}
	}

    // Generate a CSV file from the data records stored in the entries database table.
	public function create_CSV(){
		// Get the stored data records in the entries database table.
		$date=[]; $mmWave=[]; $label=[];
		list($date, $mmWave, $label) = $this->get_data_records();
		// Create the data_records.csv file.
		$filename = "assets/data_records.csv";
		$fp = fopen($filename, 'w');
		// Add the header to the CSV file.
		fputcsv($fp, ["p_1","p_2","p_3","p_4","p_5","p_6","p_7","pipe_label"]);
		// Generate rows from the retrieved data records.
		for($i=0; $i<count($date); $i++){
			$line = explode(",", $mmWave[$i]);
			array_push($line, $label[$i]);
			// Append each generated row to the CSV file as a sample.
			fputcsv($fp, $line);
		}
		// Close the CSV file.
		fclose($fp);
		// Return the CSV file name — data_records.csv.
		return $filename;
	}
}

// Define database and server settings:
$server = array(
	"name" => "localhost",
	"username" => "root",
	"password" => "",
	"database" => "pipeline_diagnostics"
);

$conn = mysqli_connect($server["name"], $server["username"], $server["password"], $server["database"]);

?>