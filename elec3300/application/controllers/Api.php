<?php

class Api extends CI_Controller {
    public function __construct() {
        parent::__construct();
    }

    public function getuserstatus(){
	     header('Access-Control-Allow-Origin: *');
	         header('Content-Type: application/json');
        $username = $_GET['username'];
        $user = $this->db->select('username, coins, hunger, exp, energy, emotion, status, action')->from('users')->where('username', $username)
        ->get()->row();


        switch($user->action){
            case 'feed':
                $user->action = 'f';
                break;
            case 'sleep':
                $user->action = 's';
                break;
            case 'play':
                $user->action = 'p';
                break;
            case 'writing':
                $user->action = 'w';
                break;
        }

        switch($user->status){
            case 'idle':
                $user->status = 'i';
                break;
            case 'hungry':
                $user->status = 'h';
                break;
            case 'sick':
                $user->status = 's';
                break;
        }


        $response = [
           'result' => "OK",
           'username' => "$user->username",
           'coins' => sprintf("%03d", $user->coins),
           'hunger' => sprintf("%03d", $user->hunger), 
           'exp' => sprintf("%03d", $user->exp),
           'energy' => sprintf("%03d", $user->energy),
           'emotion' => "$user->emotion",
           'status' => "$user->status",
           'action' => "$user->action"
        ];

		
	if ($user->action[0] == "f") {
            $this->db->where('username',$user->username);
            $this->db->update('users', ['action' => null]);
            
        }
        

        // Reset feed status if it exists
       
        echo json_encode($response);
    }
}
