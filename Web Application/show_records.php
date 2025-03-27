<?php

include_once "assets/class.php";

// Define the new 'wave' object:
$wave = new _main();
$wave->__init__($conn);

// Obtain all data records from the entries database table and print them as table rows.
$date=[]; $mmWave=[]; $label=[];
list($date, $mmWave, $label) = $wave->get_data_records();
$records = "<tr><th>Date</th><th>mmWave</th><th>Label</th></tr>";
for($i=0; $i<count($date); $i++){
	$records .= '<tr class="'.$label[$i].'">
				  <td>'.$date[$i].'</td>
				  <td style="word-break:break-all;">'.$mmWave[$i].'</td>
				  <td>'.$label[$i].'</td>
			    </tr>
			    ';   
}

// Fetch all model detection results with the assigned detection image name from the detections database table and display them as table rows.
$date_R=[]; $mmWave_R=[]; $class=[]; $img=[];
list($date_R, $mmWave_R, $class, $img) = $wave->get_model_results();
$results = "<tr><th>Date</th><th>mmWave</th><th>Model Prediction</th><th>IMG</th></tr>";
for($i=0; $i<count($date_R); $i++){
	$results .= '<tr class="'.$class[$i].'">
				  <td>'.$date_R[$i].'</td>
				  <td style="word-break:break-all;">'.$mmWave_R[$i].'</td>
				  <td>'.$class[$i].'</td>
				  <td><img src="detections/images/'.$img[$i].'"/></td>
			    </tr>
			    ';   
}

// Create a JSON object from the generated table rows consisting of the obtained data records and model detection results.
$result = array("records" => $records, "results" => $results);
$res = json_encode($result);

// Return the recently generated JSON object.
echo($res);

?>