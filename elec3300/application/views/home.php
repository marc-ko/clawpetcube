<?php
defined('BASEPATH') OR exit('No direct script access allowed');
?>

<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="utf-8">
	<title>VPetCube Dashboard</title>
	<link href="assets/css/bootstrap.css" rel="stylesheet">
	<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.15.4/css/all.min.css">
	<style>
		.progress {
			height: 10px;
		}
		.hunger-bar { background-color: #ff8c42; }
		.emotion-bar { background-color: #4a90e2; }
		.energy-bar { background-color: #42d17e; }
		.pet-image {
			background-color: #f8f9fa;
			border-radius: 8px;
			padding: 20px;
			max-width: 300px;
			margin: 20px auto;
		}
		.action-button {
			min-width: 100px;
		}
		.status-card {
			background-color: #fff;
			border-radius: 8px;
			padding: 20px;
			box-shadow: 0 2px 4px rgba(0,0,0,0.1);
		}
		.modal-backdrop.show {
			opacity: 0.8;
		}
		#welcomeModal .modal-content {
			border-radius: 15px;
			border: none;
		}
		#welcomeModal .modal-header {
			border-bottom: none;
			padding: 2rem 2rem 1rem;
		}
		#welcomeModal .modal-body {
			padding: 1rem 2rem;
		}
		#welcomeModal .modal-footer {
			border-top: none;
			padding: 1rem 2rem 2rem;
		}
		#welcomeModal .form-control-lg {
			padding: 1rem;
			font-size: 1.1rem;
			border-radius: 10px;
		}
		.activity-btn {
			text-align: left;
			padding: 1rem 1.5rem;
			transition: all 0.3s ease;
		}
		.activity-btn:hover {
			transform: translateY(-2px);
			box-shadow: 0 4px 8px rgba(0,0,0,0.1);
		}
		.activity-btn small {
			font-size: 0.85rem;
		}
		#activity-timer-container {
			text-align: center;
			padding: 20px;
			background: white;
			border-radius: 8px;
			box-shadow: 0 2px 4px rgba(0,0,0,0.1);
		}
		.timer-display {
			font-size: 3rem;
			font-weight: bold;
			color: #4a4a4a;
		}
		.current-activity-name {
			color: #666;
			font-weight: 500;
		}
		#stop-activity {
			transition: all 0.3s ease;
		}
		#stop-activity:hover {
			transform: scale(1.05);
			box-shadow: 0 2px 4px rgba(220, 53, 69, 0.2);
		}
	</style>
</head>


<body class="bg-light">
	<?php $this->load->view('nav'); ?>

	<div class="container mt-4">
		<h1 class="mb-4">Welcome Back! 👋 <?php echo $user->username; ?></h1>
		<p class="text-muted">Here's your pet's current status</p>

		<div class="row mb-4">
			<div class="col-md-4">
				<div class="status-card mb-3">
					<small class="text-muted">Hunger</small>
					<h2><?php echo $user->hunger; ?>%</h2>
					<div class="progress">
						<div class="progress-bar hunger-bar" style="width: <?php echo $user->hunger; ?>%"></div>
					</div>
				</div>
			</div>
			<div class="col-md-4">
				<div class="status-card mb-3">
					<small class="text-muted">Emotion</small>
					<h2><?php echo $user->emotion; ?>%</h2>
					<div class="progress">
						<div class="progress-bar emotion-bar" style="width: <?php echo $user->emotion; ?>%"></div>
					</div>
				</div>
			</div>
			<div class="col-md-4">
				<div class="status-card mb-3">
					<small class="text-muted">Energy</small>
					<h2><?php echo $user->energy; ?>%</h2>
					<div class="progress">
						<div class="progress-bar energy-bar" style="width: <?php echo $user->energy; ?>%"></div>
					</div>
				</div>
			</div>
		</div>

		<div class="row">
			<div class="col-md-8">
				<div class="text-center">
					<div class="pet-image">
						<img src="assets/pets/<?php echo $user->status; ?>.png" alt="Pet" class="img-fluid">
					</div>
					<div id="activity-timer-container" class="mt-3 mb-3" style="display: none;">
						<h3 class="current-activity-name mb-2"></h3>
						<div class="timer-display h1">00:00</div>
						<div class="progress mt-2 mb-3" style="height: 10px;">
							<div class="progress-bar progress-bar-striped progress-bar-animated" role="progressbar" style="width: 100%"></div>
						</div>
						<button id="stop-activity" class="btn btn-danger">
							<i class="fas fa-stop me-2"></i>Stop Activity
						</button>
					</div>
					<div class="mt-3">
						<button class="btn btn-primary action-button mx-2">Actions</button>
						<button class="btn btn-warning action-button mx-2">Feed</button>
					</div>
					<p class="mt-3 text-muted">Active and playful ✨</p>
				</div>
			</div>
			<div class="col-md-4">
				<div class="status-card">
					<h3 class="mb-4">Pet Status</h3>
					
					<div class="mb-3">
						<small class="text-muted">Emotion</small>
						<h4><?php echo $user->emotion; ?>%</h4>
						<div class="progress">
							<div class="progress-bar emotion-bar" style="width: <?php echo $user->emotion; ?>%"></div>
						</div>
					</div>

					<div class="mb-3">
						<small class="text-muted">Energy</small>
						<h4><?php echo $user->energy; ?>%</h4>
						<div class="progress">
							<div class="progress-bar energy-bar" style="width: <?php echo $user->energy; ?>%"></div>
						</div>
					</div>

					<div class="mb-4">
						<small class="text-muted">Hunger</small>
						<h4><?php echo $user->hunger; ?>%</h4>
						<div class="progress">
							<div class="progress-bar hunger-bar" style="width: <?php echo $user->hunger; ?>%"></div>
						</div>
					</div>

					<div class="mb-3">
						<small class="text-muted">Experience</small>
						<h4><?php echo $user->exp; ?> XP</h4>
						<small class="text-success">+25 today</small>
					</div>

					<div>
						<small class="text-muted">Coins</small>
						<h4><?php echo $user->coins; ?> 🪙</h4>
						<small class="text-success">+10 today</small>
					</div>
				</div>
			</div>
		</div>
	</div>

	<div class="modal fade" id="welcomeModal" data-bs-backdrop="static" data-bs-keyboard="false" tabindex="-1" aria-labelledby="welcomeModalLabel" aria-hidden="true">
		<div class="modal-dialog modal-dialog-centered">
			<div class="modal-content">
				<div class="modal-header">
					<h5 class="modal-title" id="welcomeModalLabel">Welcome to VPetCube! 🐾</h5>
				</div>
				<div class="modal-body">
					<div id="loginForm">
						<p class="text-muted h4 mb-4">Please enter your name to continue</p>
						<div class="form-group">
							<input type="text" class="form-control form-control-lg" id="username" placeholder="Your name">
						</div>
						<div class="mt-4" id="loginMessage"></div>
					</div>
				</div>
				<div class="modal-footer">
					<button type="button" class="btn btn-primary" id="continueBtn">Continue</button>
				</div>
			</div>
		</div>
	</div>

	<div class="modal fade" id="activitiesModal" tabindex="-1" aria-labelledby="activitiesModalLabel" aria-hidden="true">
		<div class="modal-dialog modal-dialog-centered">
			<div class="modal-content">
				<div class="modal-header">
					<h5 class="modal-title" id="activitiesModalLabel">Choose an Activity 🎯</h5>
					<button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
				</div>
				<div class="modal-body">
					<div class="d-grid gap-3">
						<button class="btn btn-lg btn-outline-primary activity-btn" data-activity="writing" data-duration="600">
							<i class="fas fa-pencil-alt me-2"></i> Writing
							<small class="d-block text-muted mt-1">Practice writing skills (+10 EXP)</small>
						</button>
						
						<button class="btn btn-lg btn-outline-warning activity-btn" data-activity="debug" data-duration="1800">
							<i class="fas fa-bug me-2"></i> Cleaning
							<small class="d-block text-muted mt-1">Fix some bugs (+15 EXP)</small>
						</button>
						
						<button class="btn btn-lg btn-outline-info activity-btn" data-activity="sleep" data-duration="3600">
							<i class="fas fa-moon me-2"></i> Sleep
							<small class="d-block text-muted mt-1">Rest to recover energy (+30 Energy)</small>
						</button>
					</div>
				</div>
			</div>
		</div>
	</div>

	<!-- Feed Modal -->
	<div class="modal fade" id="feedModal" tabindex="-1" aria-labelledby="feedModalLabel" aria-hidden="true">
		<div class="modal-dialog modal-dialog-centered">
			<div class="modal-content">
				<div class="modal-header">
					<h5 class="modal-title" id="feedModalLabel">Feed Your Pet 🍽️</h5>
					<button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
				</div>
				<div class="modal-body">
					<div id="inventory-items" class="d-grid gap-3">
						<!-- Items will be populated dynamically -->
					</div>
				</div>
			</div>
		</div>
	</div>

	<script src="assets/js/bootstrap.bundle.min.js"></script>
	<script src="assets/js/jquery.min.js"></script>

	<script>
	document.addEventListener('DOMContentLoaded', function() {
		// Show modal on page load if no user session exists
		var welcomeModal = new bootstrap.Modal(document.getElementById('welcomeModal'));
		
		// Check if user is logged in
		$.getJSON('<?php echo base_url('pet/check_session'); ?>', function(data) {
			if (!data.logged_in) {
				welcomeModal.show();
			} else {
			if (data.username) {
					document.getElementById('welcomeModalLabel').innerText = 'Welcome Back, ' + data.username + '! 👋';
				}
			}
		}).fail(function(xhr, status, error) {
			console.error('Session check failed:', status, error);
			welcomeModal.show();
		});

		// Handle continue button click
		document.getElementById('continueBtn').addEventListener('click', function() {
			const username = document.getElementById('username').value.trim();
			const messageDiv = document.getElementById('loginMessage');

			if (username.length < 2) {
				messageDiv.innerHTML = '<div class="alert alert-danger">Please enter a valid name</div>';
				return;
			}

			fetch('<?php echo base_url('pet/login'); ?>', {
				method: 'POST',
				headers: {
					'Content-Type': 'application/json',
				},
				body: JSON.stringify({ username: username })
			})
			.then(response => response.json())
			.then(data => {
				if (data.success) {
					welcomeModal.hide();
					location.reload();
				} else {
					messageDiv.innerHTML = '<div class="alert alert-danger">' + data.error + '</div>';
				}
			}).catch(error => {
				console.error('Error:', error);
			});
		});

		// Activities Modal
		const activitiesModal = new bootstrap.Modal(document.getElementById('activitiesModal'));
		
		// Action button click handler
		document.querySelector('.action-button').addEventListener('click', function() {
			activitiesModal.show();
		});

		// Timer functionality
		let activityTimer;
		const timerContainer = document.getElementById('activity-timer-container');
		const timerDisplay = document.querySelector('.timer-display');
		const activityName = document.querySelector('.current-activity-name');
		const progressBar = document.querySelector('.progress-bar');

		const checkActivityStatus = () => {
			fetch('<?php echo base_url('pet/check_activity_status'); ?>')
			.then(response => response.json())
			.then(data => {
				if (data.active_activity) {
					startTimer(data.remaining_time, data.activity);
					showTimer(data.activity);
				} else {
					hideTimer();
				}
			});
		};

		const formatTime = (seconds) => {
			const minutes = Math.floor(seconds / 60);
			const remainingSeconds = seconds % 60;
			return `${minutes}:${remainingSeconds.toString().padStart(2, '0')}`;
		};

		const showTimer = (activity) => {
			timerContainer.style.display = 'block';
			activityName.textContent = `Selected Action in progress...`;
		};

		const hideTimer = () => {
			timerContainer.style.display = 'none';
		};

		const startTimer = (duration, activity) => {
			let timer = duration;
			const totalDuration = duration;
			
			if (activityTimer) clearInterval(activityTimer);
			
			showTimer(activity);
			
			activityTimer = setInterval(() => {
				timerDisplay.textContent = formatTime(timer);
				const progress = (timer / totalDuration) * 100;
				progressBar.style.width = `${progress}%`;
				
				if (--timer < 0) {
					clearInterval(activityTimer);
					hideTimer();
					fetch('<?php echo base_url('pet/complete_activity'); ?>')
					.then(response => response.json())
					.then(data => {
						if (data.success) {
							location.reload();
						}
					});
				}
			}, 1000);
		};

		// Activity button click handlers
		document.querySelectorAll('.activity-btn').forEach(button => {
			button.addEventListener('click', function() {
				const activity = this.dataset.activity;
				const duration = parseInt(this.dataset.duration);
				
				fetch('<?php echo base_url('pet/perform_action'); ?>/' + activity, {
					method: 'POST',
					headers: {
						'Content-Type': 'application/json',
					},
					body: JSON.stringify({ duration: duration })
				})
				.then(response => response.json())
				.then(data => {
					if (data.success) {
						activitiesModal.hide();
						startTimer(duration, activity);
						location.reload();
					} else {
						alert(data.message || 'Error performing activity');
					}
				})
				.catch(error => {
					console.error('Error:', error);
					alert('Error performing activity');
				});
			});
		});

		// Check activity status on page load
		checkActivityStatus();

		// Feed Modal
		const feedModal = new bootstrap.Modal(document.getElementById('feedModal'));

		// Feed button click handler
		document.querySelector('.btn-warning.action-button').addEventListener('click', function() {
			fetch('<?php echo base_url('pet/get_inventory'); ?>')
			.then(response => response.json())
			.then(data => {
				const inventoryContainer = document.getElementById('inventory-items');
				inventoryContainer.innerHTML = '';

				if (Object.keys(data.items).length === 0) {
					inventoryContainer.innerHTML = `
						<div class="text-center text-muted p-4">
							<i class="fas fa-box-open fa-3x mb-3"></i>
							<p>No items in inventory. Visit the shop to buy some food!</p>
							<a href="<?php echo base_url('shop'); ?>" class="btn btn-primary">Go to Shop</a>
						</div>
					`;
					return;
				}

				Object.entries(data.items).forEach(([itemId, quantity]) => {
					const item = data.itemDetails[itemId];
					if (item.hunger_change > 0) { // Only show food items
						const button = document.createElement('button');
						button.className = 'btn btn-outline-primary inventory-item';
						button.dataset.itemId = itemId;
						button.innerHTML = `
							<div class="d-flex align-items-center">
								<img src="${item.item_image}" alt="${item.item_name}" style="width: 50px; height: 50px; object-fit: contain; margin-right: 15px;">
								<div class="flex-grow-1">
									<h5 class="mb-1">${item.item_name} (${quantity})</h5>
									<div class="stats small">
										<span class="text-success">Hunger +${item.hunger_change}</span>
										${item.energy_change ? `<span class="text-primary ms-2">Energy ${item.energy_change > 0 ? '+' : ''}${item.energy_change}</span>` : ''}
										${item.emotion_change ? `<span class="text-info ms-2">Emotion ${item.emotion_change > 0 ? '+' : ''}${item.emotion_change}</span>` : ''}
									</div>
								</div>
							</div>
						`;
						inventoryContainer.appendChild(button);
					}
				});

				// Add click handlers for inventory items
				document.querySelectorAll('.inventory-item').forEach(button => {
					button.addEventListener('click', function() {
						const itemId = this.dataset.itemId;
						fetch('<?php echo base_url('pet/use_item/'); ?>' + itemId, {
							method: 'POST'
						})
						.then(response => response.json())
						.then(data => {
							if (data.success) {
								feedModal.hide();
								location.reload();
							} else {
								alert(data.message || 'Error using item');
							}
						});
					});
				});
			});
			feedModal.show();
		});

		// Add this to your existing timer JavaScript code
		const stopButton = document.getElementById('stop-activity');

		stopButton.addEventListener('click', function() {
			if (confirm('Are you sure you want to stop this activity? You will not receive any rewards.')) {
				fetch('<?php echo base_url('pet/stop_activity'); ?>', {
					method: 'POST'
				})
				.then(response => response.json())
				.then(data => {
					if (data.success) {
						clearInterval(activityTimer);
						hideTimer();
						location.reload();
					}
				});
			}
		});
	});
	</script>
</body>
</html>
