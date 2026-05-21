<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Shop extends CI_Controller {
    
    public function __construct() {
        parent::__construct();
        $this->load->library('session');
        $this->load->database();
    }
    
    public function index() {
        $user_id = $this->session->userdata('user_id');
        if (!$user_id) {
            redirect('pet');
        }
        
        $data['items'] = $this->db->get('shop')->result();
        $user = $this->db->get_where('users', ['id' => $user_id])->row();
        $data['user_coins'] = $user->coins;
        
        $this->load->view('shop', $data);
    }
    
    public function buy_item($item_id) {
        $user_id = $this->session->userdata('user_id');
        if (!$user_id) {
            echo json_encode(['success' => false, 'message' => 'Please login first']);
            return;
        }
        
        $response = $this->purchase_item($user_id, $item_id);
        echo json_encode($response);
    }

    private function purchase_item($user_id, $item_id) {
        // Get item details
        $item = $this->db->get_where('shop', ['id' => $item_id])->row();
        if (!$item) {
            return ['success' => false, 'message' => 'Item not found'];
        }
        
        // Get user details
        $user = $this->db->get_where('users', ['id' => $user_id])->row();
        if (!$user) {
            return ['success' => false, 'message' => 'User not found'];
        }
        
        // Check if user has enough coins
        if ($user->coins < $item->item_price) {
            return ['success' => false, 'message' => 'Not enough coins'];
        }
        
        // Update user's coins
        $new_coins = $user->coins - $item->item_price;
        
        // Get current items or initialize empty array
        $current_items = json_decode($user->items, true) ?: [];
        
        // Add new item to inventory
        if (!isset($current_items[$item_id])) {
            $current_items[$item_id] = 0;
        }
        $current_items[$item_id]++;
        
        // Update user record
        $this->db->where('id', $user_id);
        $this->db->update('users', [
            'coins' => $new_coins,
            'items' => json_encode($current_items)
        ]);
        
        // Record the purchase in activities
        $activity_data = [
            'user_id' => $user_id,
            'action_type' => 'gift',
            'action_details' => json_encode([
                'item_id' => $item_id,
                'item_name' => $item->item_name,
                'price' => $item->item_price
            ]),
            'coins_change' => -$item->item_price
        ];
        
        $this->db->insert('activities', $activity_data);
        
        return ['success' => true, 'message' => 'Item purchased successfully'];
    }
}
