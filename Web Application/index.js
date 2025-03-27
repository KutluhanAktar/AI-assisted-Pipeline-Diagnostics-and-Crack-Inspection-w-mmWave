// If requested, create a CSV file (data_records.csv) from the data records saved in the entries database table.
$(".records").on("click", "button", () => {
	if(confirm("💻 Download the generated CSV file!\n\n🗂 data_records.csv")){
		window.location = "./update_server.php?create_CSV=OK";
	}
});

// Every 5 seconds, retrieve the HTML table rows generated from the database table rows to display the latest collected data records
// and inform the user of the most recent model detection results on pipeline diagnostic classes.
setInterval(function(){
	$.ajax({
		url: "./show_records.php",
		type: "GET",
		success: (response) => {
			// Decode the obtained JSON object.
			const res = JSON.parse(response);
			// Assign the data record HTML table rows.
			$(".records table").html(res.records);
			// Assign the model detection HTML table rows.
			$(".results table").html(res.results);
		}
	});
}, 5000);