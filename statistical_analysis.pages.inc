<?php

//Require lib portion to get shared functions
require_once ( dirname(__FILE__) . '/statistical_analysis.lib.inc');

/*
 * Constructs return page to be rendered by the browser
 * @param 	stat_type 	section of data to display to the user (i.e weekly, posting, voting, etc.)
 * @param 	account		user account associated with the page request
 * @param	set_title	title to be constructed if required
 * return 				rendered page to display to the user 
 */
function statistical_analysis_page($stat_type, $account = NULL, $set_title = FALSE) {
	$output = '';
  	if ($account) {
  		if ($set_title) {
      		drupal_set_title(check_plain($account->name));
    	}
    	
    	//Check to see if the user has these modules installed
  		$is_activity = module_exists('activity');
		$is_facebook_status = module_exists('facebook_status');
		$is_votingapi = module_exists('votingapi');
		$is_tagadelic = module_exists('tagadelic');
		$is_community_tags = module_exists('community_tags');
		$is_taxonmy = module_exists('taxonomy');
		$is_charts = module_exists('charts');
		$is_open_flash = module_exists('open_flash_chart_api');
		
		//Calculate out the answer of whether a not a feature is available
		if ($is_tagadelic && $is_community_tags && $is_taxonmy) { $terms = true; }
		if ($is_activity && $is_facebook_status) { $statuses = true; }
		if ($is_votingapi) { $ratings = true; }
		if ($is_charts && $is_open_flash) { $charting = true; }
		
		//Messages to display to the users when a feature is not available
		$voting_message = t("Install and enable a <a href='http://drupal.org/project/fivestar'>voting module</a> to see this statistic");
		$status_message = t("Install and enable <a href='http://drupal.org/project/facebook_status'>facebook-style statuses</a> to see this statistic");
		$term_message = t("Install and enable <a href='http://drupal.org/project/community_tags'>community tags</a> to see this statistic");
    	
		if ($account == 'admin') { $account = 'any'; } //coming from the admin panel, so no user will be present
		
		switch ($stat_type) { //Begin switch
			case 'weekly': //Current Week Statistics
				
				//Counts for the current week
				$blog_count = get_counts('weekly', 'blog', $account);
		    	$comment_count = get_counts('weekly', 'comment', $account);
		    	if ($statuses) {
		    		$status_count = get_counts('weekly', 'status', $account);
		    	} else {
	    			$status_count = $status_message;
	    		}
		    	if ($ratings) {
					$vote_count = get_voting_percentages('weekly', $account);
		    	} else {
	    			$vote_count = $voting_message;
	    		}
				
				//Render returned data into HTML for display
				if ($charting) {
					$week_hold[] = array('#value' => $blog_count, '#label' => 'Blogs', '#color' => '4E2F2F');
					$week_hold[] = array('#value' => $comment_count, '#label' => 'Comments', '#color' => '4E2F2F');
					$week_hold[] = array('#value' => $status_count, '#label' => 'Statuses', '#color' => '4E2F2F');
					$output .= generate_bar_chart('Current Week', 200, 450, $week_hold);
				} else {
					$output .= t('Amount of blogs posted: ' . $blog_count);
					$output .= '<br><br>';
					$output .= t('Amount of comments posted: ' . $comment_count);	
					$output .= '<br><br>';
					$output .= t('Amount of statuses posted: ' . $status_count);		
				}
				$output .= '<br><br>';
				$output .= t('Average rating given for blog postings: ');
			  	$output .= $vote_count; 					
				break;
				
	 		case 'posting': //Posting Statistics
	 		
	 			$terms_used_in_posts = variable_get('terms_used_in_posts', 5);
				$terms_used_in_comments = variable_get('terms_used_in_comments', 5);
    	
	 			//Counts for posted related items
				$blog_count = get_counts('summary', 'blog', $account);
		    	$comment_count = get_counts('summary', 'comment', $account);
		    	if ($statuses) {
		    		$status_count = get_counts('summary', 'status', $account);
		    	} else {
	    			$status_count = $status_message;
	    		}
		    	
		    	//Get the averages for each posting day, month, year
	    		if ($charting) {
	    			$average_day = get_posting_percentages('Day', $account, true);
	    			$average_day = generate_bar_chart('Averages Per Day', 200, 450, $average_day);
					$average_month = get_posting_percentages('Month', $account, true);
					$average_month = generate_bar_chart('Averages Per Month', 200, 450, $average_month);
					$average_year = get_posting_percentages('Year', $account, true);
					$average_year = generate_bar_chart('Averages Per Year', 200, 450, $average_year);
	    		} else {
					$average_day = get_posting_percentages('Day', $account);
					$average_month = get_posting_percentages('Month', $account);
					$average_year = get_posting_percentages('Year', $account);
	    		}
				
				//Get average length of blog postings
			    $average_length_blog = get_content_length('blog', $account);
			
				//Get average length of comment postings
			    $average_length_comment = get_content_length('comment', $account);
			    
		    	$length_hold[] = array('#value' => $average_length_blog, '#label' => 'Blogs');
				$length_hold[] = array('#value' => $average_length_comment, '#label' => 'Comments');
				$length_chart = generate_bar_chart('Average Length in Words', 200, 450, $length_hold);
		    	
				//Get popular tags used on blog postings
				if ($terms) {
			    	$top_blog_terms = get_term_posting('blog',$terms_used_in_posts,$account);
				} else {
					$top_blog_terms = $term_message;
				}
			    
			    //Get popular tags used on blogs that were commented on
				if ($terms) {
			    	$top_comment_terms = get_term_posting('comment',$terms_used_in_comments,$account);
				} else {
					$top_comment_terms = $term_message;
				}
				
			    //Render returned data into HTML for display
			    if ($charting) {
			    	$count_hold[] = array('#value' => $blog_count, '#label' => 'Blogs');
					$count_hold[] = array('#value' => $comment_count, '#label' => 'Comments');
					$count_hold[] = array('#value' => $status_count, '#label' => 'Statuses');
					$output .= generate_bar_chart('Overall Postings', 200, 450, $count_hold);
			    } else {
				    $output .= t('Amount of blogs posted: ' . $blog_count);
				  	$output .= '<br><br>';
				  	$output .= t('Amount of comments posted: ' . $comment_count);
				  	$output .= '<br><br>';
				  	$output .= t('Amount of statuses posted: ' . $status_count);
			    }
    		  	$output .= '<br><br>';
    		  	$output .= $length_chart;
			  	$output .= '<br><br>';
			  	$output .= t('Averages per day:');
			  	$output .= $average_day;
			  	$output .= '<br>';
			  	$output .= t('Averages per month:');
			  	$output .= $average_month; 	
			  	$output .= '<br>';
			  	$output .= t('Averages per year:');
			  	$output .= $average_year;   	
			  	$output .= '<br>';
			  	$output .= t("Top $terms_used_in_posts terms used in postings:");
			  	$output .= $top_blog_terms;
			  	$output .= '<br>';
			  	$output .= t("Top $terms_used_in_comments terms used in postings commented on:");
			  	$output .= $top_comment_terms; 
			  	$output .= '<br>';
				break;
				
	 		case 'voting': //Voting Statistics
	 		
	 			$top_voted_blogs = variable_get('top_voted_blogs', 5);
				$top_voting_terms = variable_get('top_voting_terms', 5);
		    	
				//Get average voting for posts
				if ($ratings) {
					$vote_count = get_voting_percentages('summary', $account);
				} else {
					$vote_count	= $voting_message;
				}
			    
			    //Get top 5 voted blogs by the author
				if ($ratings) {
					$top_blogs = get_top_blogs($top_voted_blogs, $account);
				} else {
		    		$top_blogs	= $voting_message;
		   		}
				
				//Get popular tags used on blog postings
				if ($ratings && $terms) {
		    		$top_vote_terms = get_term_posting('vote',$top_voting_terms,$account);
				} else {
					$top_vote_terms = $voting_message . ' and ' . $term_message;
				}
				
		    	//Render returned data into HTML for display
				$output .= t('Average rating given for blog postings: ');
			  	$output .= $vote_count; 
			  	$output .= '<br>';				
			  	$output .= t("Top $top_voted_blogs rated blogs authored: ");
			  	$output .= $top_blogs; 
			  	$output .= '<br>';
			  	$output .= t("Top $top_voting_terms terms from postings voted on: ");
			  	$output .= $top_vote_terms;
			  	$output .= '<br>';
			  	break; 	
			  	
			case 'contribution': //Contribution Statistics
		
				$top_blog_amount = variable_get('admin_blog_contrib', 10);
				$top_comment_amount = variable_get('admin_comment_contrib', 10);
				$top_vote_amount = variable_get('admin_vote_contrib', 10);
				
				//Top users based on content types
				$top_bloggers = get_top_contributors('blog', $top_blog_amount);
				$top_commenters = get_top_contributors('comment', $top_comment_amount);
				if ($ratings) {
					$top_voters = get_top_contributors('vote', $top_vote_amount);
				} else {
					$top_voters = $voting_message;
				}
				
				//Render returned data into HTML for display
				$output .= t("Top $top_blog_amount blog contributors: ");
			  	$output .= $top_bloggers;
				$output .= t("Top $top_comment_amount comment contributors: ");
			  	$output .= $top_commenters;
				$output .= t("Top $top_vote_amount voter contributors: ");
			  	$output .= $top_voters; 
				break;
				
			case 'trending': //Trending Statistics
				//By default show the current month
				if ($charting) {
					$sql = "select unix_timestamp(start_date) as start_date,unix_timestamp(end_date) as end_date,blogs_posted,comments_posted,statuses_posted,votes_casted,vote_average from statistical_analysis where year(start_date) = year(CURDATE()) and month(start_date) = month(curdate()) or month(end_date) = month(curdate())";
					$result = db_query($sql);
					while ($row = db_fetch_object($result)) {
						$start = format_date($row->start_date, 'custom', 'm/j' );
						$end = format_date($row->end_date, 'custom', 'm/j' );
						$week = $start . '-' . $end;
						$blogs_posted = $row->blogs_posted;
						$comments_posted = $row->comments_posted;
						$statuses_posted = $row->statuses_posted;
						$votes_posted = $row->votes_casted;
						
						$blogs_holder[] = array('#value' => $blogs_posted, '#label' => $week);
						$comments_holder[] = array('#value' => $comments_posted, '#label' => $week);
						$statuses_holder[] = array('#value' => $statuses_posted, '#label' => $week);
						$votes_holder[] = array('#value' => $votes_posted, '#label' => $week);
					}
					
					//Make charts
					$output .= generate_line_chart('Blog Trends', 200, 450, $blogs_holder);
					$output .= generate_line_chart('Comment Trends', 200, 450, $comments_holder);
					if ($statuses) {
						$output .= generate_line_chart('Status Trends', 200, 450, $statuses_holder);
					}
					if ($ratings) {
						$output .= generate_line_chart('Vote Trends', 200, 450, $votes_holder);
					}
				} else {
					$output .= "Charts must be enabled for trending data";
				}

				break;
			  		
		} //End switch
		
		return '<div id="statistical_analysis">'. $output .'</div>';
  	}
}

?>
