<?php

/* Citra IT - Excelencia em TI
 * Projeto experimental de integracao do e2guardian com o captive portal no pfSense.
 * Use por sua própria conta em risco.
 * @author: luciano@citrait.com.br
 */



/*
 * Return the entire database as an association array
 */
function captive2guardian_read_db(){
    $auth_user_db = Array();
    // read the file containing the authenticated entries
    $dnsauth_content = file_get_contents("/var/unbound/dnsauth.conf");
    $dnsauth_lines = explode("\n", $dnsauth_content);
    $dnsauth_lines_count = count($dnsauth_lines);

    // iterave over each line and parse ip -> user
    for($i=0; $i<=$dnsauth_lines_count; $i++){
        //$user_txt_dns_record = "local-data: '$dnsip.$basedomain TXT \"$user,$user_group_id\"'";
        $re_result = preg_match('/local-data: \'([0-9]{1,3}-[0-9]{1,3}-[0-9]{1,3}-[0-9]{1,3}).* TXT "([a-zA-Z0-9_\.]+),(\d+)"\'/', $dnsauth_lines[$i], $matches);
        if($re_result) {
            $target_ip_addr = str_replace("-",".", $matches[1]);
            $target_username = $matches[2];
            $target_group_id = $matches[3];
            $auth_user_db[$target_ip_addr] = $target_username;
        }
    }
    return $auth_user_db;
}



/*
 * Add an entry to 'database
*/
function captive2guardian_add_entry($ip, $user){

    $c2g_db = captive2guardian_read_db();
    $keys = array_keys($c2g_db);
    $already_added = false;
    foreach($keys as $ipaddr){
        if($ipaddr == $ip){
            if($c2g_db[$ipaddr] == $user){
                $already_added = true;
            }
        }
    }
    if($already_added){
        return false;
    }

    // read default base domain from dns authplugin config file
    $dnsauthplugin_content = file_get_contents("/usr/local/etc/e2guardian/authplugins/dnsauth.conf");
    $dnsauthplugin_lines = explode("\n", $dnsauthplugin_content);
    $dnsauthplugin_lines_count = count($dnsauthplugin_lines);
    $basedomain = "home.arpa";
    for($i=0; $i<=$dnsauthplugin_lines_count; $i++) {
            $re_result = preg_match('/basedomain = "(.*)"/', $dnsauthplugin_lines[$i], $matches);
            if($re_result) {
                $basedomain = $matches[1];
            }
    }


    // get groupid for this user
    $grouplist_content = file_get_contents("/usr/local/etc/e2guardian/lists/filtergroupslist");
    $lines = explode("\n", $grouplist_content);
    $user_group_id = 1;

    // lookup the authenticated user record in filtergrouplist
    for($i=0; $i<=count($lines); $i++) {
        if(!empty($lines[$i])) {
            $user_group_array = explode("=", $lines[$i]);
            if($user_group_array[0] == $user) {
                // found target user. extract the group id from this entry
                $user_group_id = str_replace("filter", "",  $user_group_array[1]);
            }
        }
    }


    // convert user ip to dnsauth txt format
    $dnsip = str_replace(".", "-", $ip);
    $user_txt_dns_record = "local-data: '$dnsip.$basedomain TXT \"$user,$user_group_id\"'";

    $dnsauthfile = fopen("/var/unbound/dnsauth.conf", "a");
    echo "adding entry: $user_txt_dns_record\n";
    fwrite($dnsauthfile, "\n" . $user_txt_dns_record);
    fclose($dnsauthfile);

    captive2guardian_reload_unbound();


}




/*
 * Remove one entry from authentication entries
 */
function captive2guardian_remove_entry_by_ip($target_ip) {
    // read the file containing the authenticated entries
    $dnsauth_content = file_get_contents("/var/unbound/dnsauth.conf");
    $dnsauth_lines = explode("\n", $dnsauth_content);
    $dnsauth_lines_count = count($dnsauth_lines);

    // search if the ip address already present in database
    $dnsip = str_replace(".", "-", $target_ip);
    $target_ip_found = false;
    $target_ip_found_index = 0;
    for($i=0; $i<=$dnsauth_lines_count; $i++){
        if(strpos($dnsauth_lines[$i], $dnsip) !== false){
            $target_ip_found = true;
            $target_ip_found_index = $i;
            break;

        }
    }

    // if we found the target ip, then remove existing entry for this ip in the array containing the config lines
    if($target_ip_found){
        unset($dnsauth_lines[$target_ip_found_index]);
        $dnsauth_lines = array_values($dnsauth_lines);
    }


    // save dnsauth.conf entri(es)
    if($target_ip_found){
        $dnsauthfile = fopen("/var/unbound/dnsauth.conf", "w");
        fwrite($dnsauthfile, implode("\n", $dnsauth_lines) );
        fclose($dnsauthfile);
    }

    // reloading unbound dns service
    captive2guardian_reload_unbound();

}


/*
 * Remove all entries
*/
function captive2guardian_remove_all_entries(){
        $dnsauthfile = fopen("/var/unbound/dnsauth.conf", "w");
        fwrite($dnsauthfile, implode("\n", $dnsauth_lines) );
        fclose($dnsauthfile);

        // reloading unbound dns servie
        captive2guardian_reload_unbound();
}


/*
 * Reloads unbound daemon with unbound-control
*/

function captive2guardian_reload_unbound(){
    // signalling unbound to reload with SIGHUP
    //$unbound_pid = @system("ps -xa | grep unbound | grep -v grep | awk '{print $1}'");
    //@system("kill -s HUP " . $unbound_pid);
    @system("unbound-control -c /var/unbound/unbound.conf reload");
}
