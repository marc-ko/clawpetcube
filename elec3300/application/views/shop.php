<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <title>VPetCube Shop</title>
    <link href="<?php echo base_url('assets/css/bootstrap.css'); ?>" rel="stylesheet">
    <style>
        .shop-item {
            border: 1px solid #eee;
            border-radius: 8px;
            padding: 15px;
            margin-bottom: 20px;
            transition: transform 0.2s;
        }
        .shop-item:hover {
            transform: translateY(-5px);
            box-shadow: 0 4px 8px rgba(0,0,0,0.1);
        }
        .item-image {
            width: 100px;
            height: 100px;
            object-fit: contain;
            margin-bottom: 10px;
        }
        .stats-change {
            font-size: 0.9em;
            color: #666;
        }
        .positive-stat { color: #28a745; }
        .negative-stat { color: #dc3545; }
    </style>
</head>
<body class="bg-light">
    <?php $this->load->view('nav'); ?>

    <div class="container mt-4">
        <div class="d-flex justify-content-between align-items-center mb-4">
            <h1>VPetCube Shop 🛍️</h1>
            <div class="h4">Your Coins: <span class="text-primary"><?php echo $user_coins; ?> 🪙</span></div>
        </div>

        <div class="row">
            <?php foreach($items as $item): ?>
                <div class="col-md-4">
                    <div class="shop-item bg-white">
                        <img src="<?php echo $item->item_image; ?>" alt="<?php echo $item->item_name; ?>" class="item-image">
                        <h4><?php echo $item->item_name; ?></h4>
                        <p class="text-muted"><?php echo $item->item_description; ?></p>
                        
                        <div class="stats-change mb-3">
                            <?php if($item->hunger_change != 0): ?>
                                <div class="<?php echo $item->hunger_change > 0 ? 'positive-stat' : 'negative-stat'; ?>">
                                    Hunger: <?php echo $item->hunger_change > 0 ? '+' : ''; echo $item->hunger_change; ?>
                                </div>
                            <?php endif; ?>
                            
                            <?php if($item->energy_change != 0): ?>
                                <div class="<?php echo $item->energy_change > 0 ? 'positive-stat' : 'negative-stat'; ?>">
                                    Energy: <?php echo $item->energy_change > 0 ? '+' : ''; echo $item->energy_change; ?>
                                </div>
                            <?php endif; ?>
                            
                            <?php if($item->emotion_change != 0): ?>
                                <div class="<?php echo $item->emotion_change > 0 ? 'positive-stat' : 'negative-stat'; ?>">
                                    Emotion: <?php echo $item->emotion_change > 0 ? '+' : ''; echo $item->emotion_change; ?>
                                </div>
                            <?php endif; ?>
                            
                            <?php if($item->exp_change != 0): ?>
                                <div class="positive-stat">
                                    EXP: +<?php echo $item->exp_change; ?>
                                </div>
                            <?php endif; ?>
                        </div>
                        
                        <button class="btn btn-primary w-100 buy-item" 
                                data-item-id="<?php echo $item->id; ?>"
                                data-item-price="<?php echo $item->item_price; ?>">
                            Purchase for <?php echo $item->item_price; ?> 🪙
                        </button>
                    </div>
                </div>
            <?php endforeach; ?>
        </div>
    </div>

    <script src="<?php echo base_url('assets/js/jquery.min.js'); ?>"></script>
    <script>
        $(document).ready(function() {
            $('.buy-item').click(function() {
                const button = $(this);
                const itemId = button.data('item-id');
                const itemPrice = button.data('item-price');
                
                $.ajax({
                    url: '<?php echo base_url('shop/buy_item/'); ?>' + itemId,
                    method: 'POST',
                    success: function(response) {
                        const data = JSON.parse(response);
                        if(data.success) {
                            alert('Item purchased successfully!');
                            location.reload();
                        } else {
                            alert(data.message || 'Not enough coins!');
                        }
                    },
                    error: function() {
                        alert('Error occurred while purchasing item');
                    }
                });
            });
        });
    </script>
</body>
</html> 