-- phpMyAdmin SQL Dump
-- version 5.2.1deb3
-- https://www.phpmyadmin.net/
--
-- Host: localhost:3306
-- Generation Time: Dec 02, 2024 at 04:23 PM
-- Server version: 8.0.40-0ubuntu0.24.04.1
-- PHP Version: 8.3.6

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `vpetcube`
--

-- --------------------------------------------------------

--
-- Table structure for table `activities`
--

CREATE TABLE `activities` (
  `id` int NOT NULL,
  `user_id` int NOT NULL,
  `action_type` enum('feed','play','sleep','gift') NOT NULL,
  `action_details` json DEFAULT NULL,
  `coins_change` int DEFAULT '0',
  `exp_change` int DEFAULT '0',
  `hunger_change` int DEFAULT '0',
  `energy_change` int DEFAULT '0',
  `emotion_change` int DEFAULT '0',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `activities`
--

INSERT INTO `activities` (`id`, `user_id`, `action_type`, `action_details`, `coins_change`, `exp_change`, `hunger_change`, `energy_change`, `emotion_change`, `created_at`) VALUES
(1, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 01:14:38'),
(2, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 01:36:08'),
(3, 416, 'gift', '{\"price\": \"10\", \"item_id\": \"1\", \"item_name\": \"Hamburger\"}', -10, 0, 0, 0, 0, '2024-11-28 01:37:01'),
(4, 416, 'gift', '{\"price\": \"20\", \"item_id\": \"2\", \"item_name\": \"Ball\"}', -20, 0, 0, 0, 0, '2024-11-28 01:37:04'),
(5, 416, 'gift', '{\"price\": \"10\", \"item_id\": \"1\", \"item_name\": \"Hamburger\"}', -10, 0, 0, 0, 0, '2024-11-28 01:37:06'),
(6, 416, 'gift', '{\"price\": \"10\", \"item_id\": \"1\", \"item_name\": \"Hamburger\"}', -10, 0, 0, 0, 0, '2024-11-28 02:26:42'),
(7, 416, 'gift', '{\"price\": \"10\", \"item_id\": \"1\", \"item_name\": \"Hamburger\"}', -10, 0, 0, 0, 0, '2024-11-28 02:58:37'),
(8, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 04:48:27'),
(9, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 04:56:04'),
(10, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 05:06:55'),
(11, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 05:14:34'),
(12, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 05:30:52'),
(13, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 05:33:17'),
(14, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 05:33:47'),
(15, 416, 'gift', '{\"price\": \"10\", \"item_id\": \"1\", \"item_name\": \"Hamburger\"}', -10, 0, 0, 0, 0, '2024-11-28 05:50:52'),
(16, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 05:59:36'),
(17, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 06:01:00'),
(18, 416, 'gift', '{\"price\": \"10\", \"item_id\": \"1\", \"item_name\": \"Hamburger\"}', -10, 0, 0, 0, 0, '2024-11-28 06:06:55'),
(19, 416, 'gift', '{\"price\": \"30\", \"item_id\": \"3\", \"item_name\": \"Premium Food\"}', -30, 0, 0, 0, 0, '2024-11-28 06:06:57'),
(20, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 06:11:41'),
(21, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 06:15:07'),
(22, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 06:19:08'),
(23, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 06:51:34'),
(24, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 06:52:37'),
(25, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 07:05:22'),
(26, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 07:06:22'),
(27, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 07:09:47'),
(28, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 07:12:47'),
(29, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 07:41:49'),
(30, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 07:57:03'),
(31, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 07:58:35'),
(32, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 07:59:27'),
(33, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 08:00:32'),
(34, 416, 'gift', '{\"price\": \"10\", \"item_id\": \"1\", \"item_name\": \"Hamburger\"}', -10, 0, 0, 0, 0, '2024-11-28 08:01:10'),
(35, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 08:17:23'),
(36, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 08:18:22'),
(37, 416, '', '{\"message\": \"Activity stopped by user\"}', 0, 0, 0, 0, 0, '2024-11-28 08:20:19'),
(38, 416, 'gift', '{\"price\": \"10\", \"item_id\": \"1\", \"item_name\": \"Hamburger\"}', -10, 0, 0, 0, 0, '2024-11-28 08:27:46');

-- --------------------------------------------------------

--
-- Table structure for table `shop`
--

CREATE TABLE `shop` (
  `id` int NOT NULL,
  `item_name` text NOT NULL,
  `item_price` int NOT NULL,
  `item_description` text NOT NULL,
  `item_image` text NOT NULL,
  `hunger_change` int DEFAULT '0',
  `energy_change` int DEFAULT '0',
  `emotion_change` int DEFAULT '0',
  `exp_change` int DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `shop`
--

INSERT INTO `shop` (`id`, `item_name`, `item_price`, `item_description`, `item_image`, `hunger_change`, `energy_change`, `emotion_change`, `exp_change`) VALUES
(1, 'Hamburger', 10, 'A delicious meal for your pet', 'https://png.pngtree.com/png-clipart/20230216/ourmid/pngtree-juicy-burgers-with-a-transparent-background-png-image_6603069.png', 10, -10, 0, 10),
(2, 'Ball', 20, 'A fun toy for your pet', 'https://i.pinimg.com/originals/01/2d/f0/012df009c074cef852c05719b6b6a73a.png', 0, -10, 10, 15),
(3, 'Premium Food', 30, 'High quality pet food that fills them up', 'https://static.vecteezy.com/system/resources/previews/047/598/224/non_2x/beef-steak-served-in-plate-on-white-background-grilled-steak-medium-rare-png.png', 25, -5, 5, 20),
(4, 'Plush Toy', 25, 'A soft and cuddly toy', 'assets/items/plush.png', 0, -15, 15, 20),
(5, 'Energy Drink', 15, 'Gives your pet an energy boost', 'assets/items/drink.png', -5, 30, 5, 10),
(6, 'Treat Box', 40, 'A box full of various treats', 'assets/items/treats.png', 15, -5, 10, 25),
(7, 'Puzzle Toy', 35, 'A mentally stimulating toy', 'assets/items/puzzle.png', -5, -20, 20, 30),
(8, 'Vitamin Snack', 20, 'Healthy snack with vitamins', 'assets/items/vitamin.png', 5, 10, 5, 15),
(9, 'Luxury Bed', 50, 'Premium bed for better rest', 'assets/items/bed.png', 0, 40, 10, 20),
(10, 'Party Pack', 45, 'Fun party items for celebration', 'assets/items/party.png', -10, -15, 25, 35);

-- --------------------------------------------------------

--
-- Table structure for table `users`
--

CREATE TABLE `users` (
  `id` int NOT NULL,
  `user_id` varchar(100) NOT NULL,
  `username` varchar(50) NOT NULL,
  `coins` int DEFAULT '100',
  `hunger` int DEFAULT '100',
  `exp` int DEFAULT '0',
  `energy` int DEFAULT '100',
  `emotion` int DEFAULT '100',
  `items` json DEFAULT NULL,
  `status` text,
  `action` text,
  `action_start_time` timestamp NULL DEFAULT NULL,
  `action_duration` int DEFAULT NULL,
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `last_action_time` timestamp NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `users`
--

INSERT INTO `users` (`id`, `user_id`, `username`, `coins`, `hunger`, `exp`, `energy`, `emotion`, `items`, `status`, `action`, `action_start_time`, `action_duration`, `created_at`, `last_action_time`) VALUES
(416, '', 'billy', 90, 96, 80, 15, 73, '{\"2\": 1}', 'idle', NULL, NULL, NULL, '2024-11-27 23:02:17', '2024-11-27 23:02:17');

--
-- Indexes for dumped tables
--

--
-- Indexes for table `activities`
--
ALTER TABLE `activities`
  ADD PRIMARY KEY (`id`),
  ADD KEY `user_id` (`user_id`);

--
-- Indexes for table `shop`
--
ALTER TABLE `shop`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `users`
--
ALTER TABLE `users`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `user_id` (`user_id`),
  ADD UNIQUE KEY `username` (`username`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `activities`
--
ALTER TABLE `activities`
  MODIFY `id` int NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=39;

--
-- AUTO_INCREMENT for table `shop`
--
ALTER TABLE `shop`
  MODIFY `id` int NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=11;

--
-- AUTO_INCREMENT for table `users`
--
ALTER TABLE `users`
  MODIFY `id` int NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=417;

--
-- Constraints for dumped tables
--

--
-- Constraints for table `activities`
--
ALTER TABLE `activities`
  ADD CONSTRAINT `activities_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`) ON DELETE CASCADE;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
