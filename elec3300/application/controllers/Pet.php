<?php 
defined('BASEPATH') OR exit('No direct script access allowed');

class Pet extends CI_Controller {
    public function __construct() {
        parent::__construct();
        $this->load->library('session');
        $this->load->database();
    }
    public function index() {
        $user_id = $this->session->userdata('user_id');
        if (empty($user_id)) {
            $this->load->view('home');
        }

        $result = $this->db->get_where('users', ['id' => $user_id])->row();
        $this->load->view('home', ['user' => $result]);

    }

    private function uuidv4()
    {
        $data = random_bytes(16);

        $data[6] = chr(ord($data[6]) & 0x0f | 0x40); // set version to 0100
        $data[8] = chr(ord($data[8]) & 0x3f | 0x80); // set bits 6-7 to 10
            
        return vsprintf('%s%s-%s-%s-%s-%s%s%s', str_split(bin2hex($data), 4));
    }

    private function create_user($username) {
        $user_id = $this->uuidv4();

        $data = [
            'id' => $user_id,
            'username' => $username,
            'coins' => 100,
            'hunger' => 100,
            'exp' => 0,
            'energy' => 100,
            'emotion' => 100,
            'status' => 'idle',
        ];

        $this->db->insert('users', $data);
        return $user_id;
    }

    public function login() {
        $input_data = file_get_contents('php://input');
        $post = json_decode($input_data,true);
        $username = $post['username'];

        if (empty($username)) {
            echo json_encode(['error' => $post]);
            return;
        }

        $user = $this->db->get_where('users', array('username' => $username))->row();
        $user_id = $user->id;
        if(empty($user)) {
            $user_id = $this->create_user($username);
            $this->session->set_userdata('user_id', $user_id);
        }

        echo json_encode(['logged_in' => true, 'username' => $username,'user_id' => $user_id]);
    }

    public function check_session() {
        $user_id = $this->session->userdata('user_id');


        if (empty($user_id)) {
            echo json_encode(['logged_in' => false]);
            return;
        } 
        
        $result = $this->db->get_where('users', ['id' => $user_id])->row();
        if (empty($result)) {
            $this->create_user($result->username);
        }
        echo json_encode(['logged_in' => true, 'username' => $result->username]);   
    }

    public function perform_action($action) {
        $user_id = $this->session->userdata('user_id');
        if (!$user_id) {
            echo json_encode(['success' => false, 'message' => 'Please login first']);
            return;
        }

        $input = json_decode(file_get_contents('php://input'), true);
        $duration = $input['duration'] ?? 0;

        $changes = [
            'writing' => [
                'exp' => 10,
                'energy' => -5,
                'emotion' => 5,
                'duration' => 600 // 10 minutes
            ],
            'debug' => [
                'exp' => 15,
                'energy' => -10,
                'emotion' => -5,
                'duration' => 1800 // 30 minutes
            ],
            'sleep' => [
                'energy' => 30,
                'emotion' => 10,
                'exp' => 5,
                'duration' => 3600 // 60 minutes
            ]
        ];

        if (!isset($changes[$action])) {
            echo json_encode(['success' => false, 'message' => 'Invalid action']);
            return;
        }

        $this->db->trans_start();
        
        $this->db->where('id', $user_id);
        $this->db->update('users', [
	    'status' => ($action[0] == 's') ? 'sleeping':'writing',
            'action' => $action[0],
            'action_start_time' => date('Y-m-d H:i:s'),
            'action_duration' => $duration
        ]);

        $this->db->trans_complete();

        echo json_encode(['success' => true]);
    }

    public function check_activity_status() {
        $user_id = $this->session->userdata('user_id');
        $user = $this->db->get_where('users', ['id' => $user_id])->row();

        if (!$user || !$user->action_start_time || !$user->action_duration) {
            echo json_encode(['active_activity' => false]);
            return;
        }

        $start_time = strtotime($user->action_start_time);
        $current_time = time();
        $elapsed_time = $current_time - $start_time;
        $remaining_time = max(0, $user->action_duration - $elapsed_time);

        echo json_encode([
            'active_activity' => $remaining_time > 0,
            'activity' => $user->status,
            'remaining_time' => $remaining_time
        ]);
    }

    public function complete_activity() {
        $user_id = $this->session->userdata('user_id');
        $user = $this->db->get_where('users', ['id' => $user_id])->row();

        // Apply activity changes
        $changes = [
            'writing' => ['exp' => 10, 'energy' => -5, 'emotion' => 5],
            'debug' => ['exp' => 15, 'energy' => -10, 'emotion' => -5],
            'sleep' => ['energy' => 30, 'emotion' => 10, 'exp' => 5]
        ];

        $activity_changes = $changes[$user->status] ?? [];
        
        $this->db->trans_start();
        
        foreach ($activity_changes as $stat => $change) {
            $this->db->set($stat, "GREATEST(0, LEAST(100, $stat + $change))", false);
        }
        
        $this->db->set('status', 'idle');
        $this->db->set('action_start_time', null);
        $this->db->set('action_duration', null);
        $this->db->where('id', $user_id);
        $this->db->update('users');

        $this->db->trans_complete();

        echo json_encode(['success' => true]);
    }

    public function get_inventory() {
        $user_id = $this->session->userdata('user_id');
        $user = $this->db->get_where('users', ['id' => $user_id])->row();
        
        $items = json_decode($user->items, true) ?: [];
        
        // Get details for all items
        $itemDetails = [];
        if (!empty($items)) {
            $this->db->where_in('id', array_keys($items));
            $itemDetails = $this->db->get('shop')->result();
            $itemDetails = array_column($itemDetails, null, 'id');
        }
        
        echo json_encode([
            'success' => true,
            'items' => $items,
            'itemDetails' => $itemDetails
        ]);
    }

    public function use_item($item_id) {
        $user_id = $this->session->userdata('user_id');
        $user = $this->db->get_where('users', ['id' => $user_id])->row();
        
        $items = json_decode($user->items, true) ?: [];
        
        if (!isset($items[$item_id]) || $items[$item_id] <= 0) {
            echo json_encode(['success' => false, 'message' => 'Item not found in inventory']);
            return;
        }
        
        $item = $this->db->get_where('shop', ['id' => $item_id])->row();
        
        $this->db->trans_start();
        
        // Update user stats
        $updates = [];
        foreach (['hunger', 'energy', 'emotion'] as $stat) {
            $change_field = $stat . '_change';
            if ($item->$change_field != 0) {
                $updates[$stat] = "GREATEST(0, LEAST(100, $stat + {$item->$change_field}))";
            }
        }
        
        if (!empty($updates)) {
            $this->db->set($updates, '', false);
        }
        
        // Set the feeding status
        $this->db->set('action', 'f');
        
        // Remove one item from inventory
        $items[$item_id]--;
        if ($items[$item_id] <= 0) {
            unset($items[$item_id]);
        }
        
        $this->db->set('items', json_encode($items));
        $this->db->where('id', $user_id);
        $this->db->update('users');
        
        $this->db->trans_complete();
        
        echo json_encode(['success' => true]);
    }

    public function stop_activity() {
        $user_id = $this->session->userdata('user_id');
        
        $this->db->trans_start();
        
        // Reset user's activity status
        $this->db->where('id', $user_id);
        $this->db->update('users', [
		'status' => 'idle',
		'action'=>null,
            'action_start_time' => null,
            'action_duration' => null
        ]);
        
        // Record the stopped activity
        $this->db->insert('activities', [
            'user_id' => $user_id,
            'action_type' => 'stopped',
            'action_details' => json_encode([
                'message' => 'Activity stopped by user'
            ]),
            'coins_change' => 0,
            'exp_change' => 0
        ]);
        
        $this->db->trans_complete();
        
        echo json_encode(['success' => true]);
    }
}

