<?php 

/*
 * Return counts for different content types and intervals
 * @param	interval	when to pull data
 * @param	content		type of data to pull
 * @param	account		account associated with the data
 * return				count based on passed in params
 */
function get_counts($interval, $content, $account='any') {

	//Variables set by hook_cron to measure time - used in weekly pullings
	$start_date = variable_get('statistical_analysis_current_week', '0');
	$end_date = variable_get('statistical_analysis_next_week', '0');
	
	//Dynamically construct SQL based on based in parameters
	if ($content == 'blog') {
		$sql = "SELECT COUNT(nid) AS amount FROM {node} WHERE type = 'blog'";
		if ($account != 'any') {
			$sql .= " AND uid = '$account->uid'";
		}
		if ($interval == 'weekly') {
			$sql .= " AND DATE(FROM_UNIXTIME(created)) BETWEEN '$start_date' AND '$end_date'";
		}
	} else if ($content == 'comment') {
	  	$sql = "SELECT COUNT(*) AS amount FROM {comments} where status = '0'";
		if ($account != 'any') {
			$sql .= " AND uid = '$account->uid'";
		}
		if ($interval == 'weekly') {
			$sql .= " AND DATE(FROM_UNIXTIME(timestamp)) BETWEEN '$start_date' AND '$end_date'";
		}		  	
	} else if ($content == 'status') {
		$sql = "SELECT COUNT(aid) AS amount FROM {activity} WHERE module = 'facebook_status'";
		if ($account != 'any') {
			$sql .= " AND uid = '$account->uid'";
		}
		if ($interval == 'weekly') {
			$sql .= " AND DATE(FROM_UNIXTIME(created)) BETWEEN '$start_date' AND '$end_date'";
		}	
	} else if ($content == 'vote') {
		$sql = "SELECT COUNT(value) AS amount FROM {votingapi_vote} WHERE content_type = 'node'";
		if ($account != 'any') {
			$sql .= " AND uid = '$account->uid'";
		}
		if ($interval == 'weekly') {
			$sql .= " AND DATE(FROM_UNIXTIME(timestamp)) BETWEEN '$start_date' AND '$end_date'";
		}
	} else if ($content == 'read') {
		return "0";
	} else {
		return "Bad content type called";
	}
	
	$result = db_query($sql);
	while ($row = db_fetch_object($result)) {
		$count = $row->amount;
	}
	
	return $count;
}

/*
 * Return top contributors of different content types
 * @param	content		type of data to pull
 * @param	amount		how many entries to return
 * return				table of top contributors for different content types
 */
function get_top_contributors($content, $amount) {
	
	$top_contrib = array();
	$sql = "SELECT uid,name FROM {users}";
	$result = db_query($sql);
	
	//Select all users from the Drupal installation
	while ($row = db_fetch_object($result)) {
		$user = $row->name; //Store the name
		$uid = $row->uid; //Store the ID
		
		//Pick and construct SQL based on content type
		if ($content == 'blog') {
			$sql = "SELECT COUNT(nid) AS count FROM {node} WHERE type = 'blog' AND uid = '$row->uid'";
		} else if ($content == 'comment') {
			$sql = "SELECT COUNT(nid) AS count FROM {comments} WHERE uid = '$row->uid'";
		} else if ($content == 'vote') {
			$sql = "SELECT COUNT(content_id) AS count FROM {votingapi_vote} WHERE uid = '$row->uid'";
		} else {
			return "Content type not valid";
		}
		
		$sub_result = db_query($sql);
		//Throw all users with associated count into an array for later sorting
		while ($sub_row = db_fetch_object($sub_result)) {
			$top_contrib["<a href='" . $base_url . "/?q=user/" . $uid . "'>" . $user . "</a>"] = $sub_row->count;
		}
	}
	
	if (count($top_contrib) == 0) {
		$top_rows[] = array(array('data' => t('Top contributor information not available'),'colspan' => '5'));	
	} else {
		array_multisort($top_contrib, SORT_DESC, 0); //Sort highest to lowest
		$top = array_slice($top_contrib, 0, $amount); //Keep only the amount passed in
	  	$top_header = array(t('User'), t('Contribution'));
		foreach ($top as $key => $value) {
			$top_rows[] = array(array('data' => t($key)),array('data' => t($value)));	
		}
	}
	
	return theme('table', $top_header, $top_rows); 
}

/*
 * Gets the average content length of a content type
 * @param	content		type of data to pull
 * @param	account		account associated with the content
 * return				average length of content in words
 */
function get_content_length($content, $account='any') {
	
	$length_count = 0;
	$total_length = 0;
	
	//Dynamically construct the SQL based on passed in params
	if ($content == 'blog') {
		$sql = "SELECT LENGTH(body) - LENGTH(REPLACE(body, ' ', ''))+1 AS length FROM {node_revisions}";
		if ($account != 'any') {
			$sql .= " WHERE uid = '$account->uid'";
		}
	} else if ($content == 'comment') {
		$sql = "SELECT LENGTH(comment) - LENGTH(REPLACE(comment, ' ', ''))+1 AS length FROM {comments}";
		if ($account != 'any') {
			$sql .= " WHERE uid = '$account->uid'";
		}
	} else {
		return "Content type not valid";
	}
	
	$result = db_query($sql);
	while ($row = db_fetch_object($result)) {
		$total_length = $total_length + $row->length;
		$length_count++;
	}
	
    if ($length_count == 0) {
    	$average_length = "0";
    } else {
    	$average_length = round($total_length/$length_count,2);
 	}
 	
 	return $average_length;
}

/*
 * Return top terms from a content type
 * @param	content		type of data to pull
 * @param	amount		how many entries to return
 * @param	account		account associated with the content
 * return				table of top contributors for different content types
 */
function get_term_posting($content, $amount, $account='any') {
	
	//Dynamically build SQL based on passed in params
	if ($content == 'blog') {
		$sql = "SELECT nid FROM {node}";
		if ($account != 'any') {
			$sql .= " WHERE uid = '$account->uid'";
		}
	} else if ($content == 'comment') {
		$sql = "SELECT nid from {comments}";
		if ($account != 'any') {
			$sql .= " WHERE uid = '$account->uid'";
		}
	} else if ($content == 'vote') {
		    $sql = "SELECT vv.content_id as nid, vv.value FROM {votingapi_vote} vv, {node} n WHERE vv.content_id = n.nid";
		    if ($account != 'any') {
				$sql .= " AND vv.uid = '$account->uid'";
			}
	} else {
		return "Content type not valid";
	}
	
	$terms = array();
	$result = db_query($sql);
	//Loop through all content for a type and get the counts on terms associated with each
	while ($row = db_fetch_object($result)) {
		$sql = "SELECT td.name,td.tid FROM {term_data} td, {term_node} tn WHERE td.tid = tn.tid AND tn.nid = '$row->nid'";
		$sub_result = db_query($sql);
		while ($sub_row = db_fetch_object($sub_result)) {
			$count = $terms["<a href='" . $base_url . "/?q=taxonomy/term/" . $sub_row->tid . "'>" . $sub_row->name . "</a>"];
			$term_total = $count + 1;
			$terms["<a href='" . $base_url . "/?q=taxonomy/term/" . $sub_row->tid . "'>" . $sub_row->name . "</a>"] = $term_total;
		}
	}
	if (count($terms) == 0) {
		$term_rows[] = array(array('data' => t("No term data is available"),'colspan' => '5'));	
	} else {
		array_multisort($terms, SORT_DESC, 0); //Sort highest to lowest
		$top = array_slice($terms, 0, $amount); //Keep only the amount passed in
	  	$term_header = array(t('Term'), t('Times Used'));
		foreach ($top as $key => $value) {
			$term_rows[] = array(array('data' => t($key)),array('data' => t($value)));
		}
	}
	
	return theme('table', $term_header, $term_rows);
}

/*
 * Take in all posting realted information and pull averages for different periods
 * @param	interval	period in which to pull from
 * @param	account		account associated with the data
 * return 				table of differing percentages based on content
 */
function get_posting_percentages($interval, $account = 'any', $chart = false) {

	//Turn the passed in interval into a proper date code
	if ($interval == 'Day') { $date_code = 'D'; }
	if ($interval == 'Month') { $date_code = 'M'; }
	if ($interval == 'Year') { $date_code = 'Y'; }	

	$complete_total = 0;
	$posts = array();

	if ($account != 'any') {
		$sql = "SELECT created as created FROM {node} WHERE type = 'blog' AND uid = '$account->uid' UNION SELECT timestamp as created FROM {comments} WHERE uid = '$account->uid'";
	} else {
    	$sql = "SELECT created as created FROM {node} WHERE type = 'blog' UNION SELECT timestamp as created FROM {comments}";
	}
    $result = db_query($sql);
   	while ($row = db_fetch_object($result)) {
    	$time = strtoupper(format_date($row->created,'custom',$date_code));
		$count = $posts[$time];
		$total = $count + 1;
		$posts[$time] = $total;
		$complete_total++;
    }
    
    if ($chart) {
        foreach ($posts as $key => $value) {
			$core_hold[] = array('#value' => t(round($value/$complete_total *100,2)), '#label' => $key);
    	}
    	return $core_hold;
    } else {
		$time_header = array(t($interval), t('Percentage'));
		foreach ($posts as $key => $value) {
			$time_rows[] = array(array('data' => t($key)),array('data' => t(round($value/$complete_total *100,2) . '%'),'colspan' => '5'));	
		}
		return theme('table',$time_header,$time_rows);
    }
	

}

/*
 * Calculate voting percentage based on period and account
 * @param	interval	period in which to pull from
 * @param	account		account associated with the data
 * return 				table with amount of votes casted and the average rating given
 */
function get_voting_percentages($interval, $account = 'any') {
	
	//Variables set by hook_cron to measure time - used in weekly pullings
	$start_date = variable_get('statistical_analysis_current_week', '0');
	$end_date = variable_get('statistical_analysis_next_week', '0');
	$length_count = 0;
	$total_length = 0;
	$sql = "SELECT value FROM {votingapi_vote} WHERE content_type = 'node'";
	
	if ($account != 'any') {
		$sql .= " AND uid = '$account->uid'";
	}
	if ($interval == 'weekly') {
		$sql .= " AND DATE(FROM_UNIXTIME(timestamp)) BETWEEN '$start_date' AND '$end_date'";
	}
	
	$result = db_query($sql);
	while ($row = db_fetch_object($result)) {
		$total_length = $total_length + $row->value;
		$length_count++;
    }
      
    if ($length_count == 0) {
    	$vote_header = array();
    	$vote_rows[] = array(array('data' => t("No votes were recorded for this user this week")));	  	
    } else {
    	$vote_header = array(t('Average'), t('Vote Count'));
      	$vote_rows[] = array(array('data' => t(round($total_length/$length_count,0) . '%')),array('data' => t($length_count)));	  	
    }
    
    return theme('table', $vote_header,$vote_rows);
}

/*
 * Calculate top blogs based on number of votes and their averages
 * @param	$amount		amount of blogs to include
 * @param	account		account associated with the data
 * return 				table with top rated blogs
 */
function get_top_blogs($amount, $account = 'any') {
   	$entries = array();
    $sql = "SELECT nid FROM {node}";
	if ($account != 'any') {
	    $sql .= " WHERE uid = '$account->uid'";
	}

	$result = db_query($sql);
	while ($row = db_fetch_object($result)) {
		$sql = "SELECT SUM(vv.value)/COUNT(vv.value) AS value,n.title FROM {votingapi_vote} vv, {node} n WHERE vv.content_id = n.nid AND n.nid = '$row->nid'";
		if ($account != 'any') {
			$sql .= " AND n.uid = '$account->uid' ";
		}
		$sql .= " GROUP BY n.title";

		$sub_result = db_query($sql);
		$nid = $row->nid;

		while ($sub_row = db_fetch_object($sub_result)) {
			$entries["<a href='" . $base_url . "/?q=node/" . $nid . "'>" . $sub_row->title . "</a>"] = $sub_row->value;
		}
	}
	if (count($entries) == 0) {
		$rated_blog_rows[] = array(array('data' => t("No entries authored by this user have been voted on"),'colspan' => '5'));	
	} else {
		array_multisort($entries, SORT_DESC, 0); //Sort highest to lowest
		$top_5 = array_slice($entries, 0, $amount); //Keep only the amount passed in
  		$entries_header = array(t('Title'), t('Average Vote'));
		foreach ($top_5 as $key => $value) {
			$rated_blog_rows[] = array(array('data' => t($key)),array('data' => t($value . '%')));	
		}
	}
	
	return theme('table', $entries_header, $rated_blog_rows);
}

/*
 * Create a bar chart with some basic information
 * @param	title		title of the chart
 * @param	height		height of the chart
 * @param	width		width of the chart
 * @param	data		date to display in the chart
 * return 				return a chart
 */
function generate_bar_chart($title,$height,$width,$data) {
	$chart = array(
		'#title'	=> $title,
		'#plugin'	=> 'openflashchart',
		'#type'		=> 'vbar2D',
		'#height'	=> $height, //in pixels
		'#width'	=> $width, //in pixels
		'#color'	=> array('#background' => 'FFFFFF'),

		$data,
	);
	
	return charts_chart($chart);
}

/*
 * Create a line chart with some basic information
 * @param	title		title of the chart
 * @param	height		height of the chart
 * @param	width		width of the chart
 * @param	data		date to display in the chart
 * return 				return a chart
 */
function generate_line_chart($title,$height,$width,$data) {
	$chart = array(
		'#title'	=> $title,
		'#plugin'	=> 'openflashchart',
		'#type'		=> 'line2D',
		'#height'	=> $height, //in pixels
		'#width'	=> $width, //in pixels
		'#color'	=> array('#background' => 'FFFFFF'),

		$data,
	);
	
	return charts_chart($chart);
}

?>