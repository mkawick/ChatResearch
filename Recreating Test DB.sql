SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='TRADITIONAL,ALLOW_INVALID_DATES';

CREATE SCHEMA IF NOT EXISTS `playdek` DEFAULT CHARACTER SET utf8 ;
USE `playdek` ;

-- -----------------------------------------------------
-- Table `playdek`.`admin_account`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`admin_account` (
  `key` VARCHAR(45) NOT NULL,
  `value` VARCHAR(45) NULL DEFAULT '0',
  PRIMARY KEY (`key`))
ENGINE = MyISAM
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`admin_contact`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`admin_contact` (
  `key` VARCHAR(45) NOT NULL,
  `value` VARCHAR(45) NULL DEFAULT NULL,
  PRIMARY KEY (`key`),
  UNIQUE INDEX `key_UNIQUE` (`key` ASC))
ENGINE = MyISAM
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`chat_channel`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`chat_channel` (
  `id` INT(8) UNSIGNED NOT NULL AUTO_INCREMENT,
  `name` VARCHAR(45) NULL DEFAULT NULL,
  `uuid` VARCHAR(16) NULL DEFAULT NULL,
  `is_active` TINYINT(1) NULL DEFAULT '1',
  `max_num_users` INT(11) NULL DEFAULT '32',
  `game_type` TINYINT(2) NULL DEFAULT '0',
  `game_instance_id` INT(1) NULL DEFAULT '0',
  `date_created` TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP,
  `date_expired` TIMESTAMP NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`id`))
ENGINE = InnoDB
AUTO_INCREMENT = 307
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`chat_message`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`chat_message` (
  `id` INT(8) UNSIGNED NOT NULL AUTO_INCREMENT,
  `text` VARCHAR(255) NOT NULL COMMENT 'utf8 text',
  `user_id_sender` VARCHAR(16) NOT NULL,
  `user_id_recipient` VARCHAR(16) NULL DEFAULT NULL,
  `chat_channel_id` VARCHAR(16) NULL DEFAULT NULL,
  `timestamp` TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP,
  `game_turn` INT(8) UNSIGNED NULL DEFAULT NULL,
  PRIMARY KEY (`id`))
ENGINE = InnoDB
AUTO_INCREMENT = 2036
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`config`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`config` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `key` VARCHAR(64) NULL DEFAULT NULL,
  `value` VARCHAR(45) NULL DEFAULT NULL,
  `category` VARCHAR(45) NULL DEFAULT NULL COMMENT 'An optional field by which we can sort per game, project, etc',
  `notes` VARCHAR(255) NULL DEFAULT NULL COMMENT 'any notes',
  PRIMARY KEY (`id`))
ENGINE = MyISAM
AUTO_INCREMENT = 9
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`users`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`users` (
  `user_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `user_name` VARCHAR(32) NULL DEFAULT NULL,
  `user_name_match` VARCHAR(32) NULL DEFAULT NULL,
  `user_pw_hash` BIGINT(20) UNSIGNED NULL DEFAULT NULL,
  `user_email` VARCHAR(64) NULL DEFAULT NULL,
  `user_gamekit_id` VARCHAR(32) NULL DEFAULT NULL,
  `user_gamekit_hash` BIGINT(20) UNSIGNED NOT NULL,
  `user_creation_date` DATETIME NOT NULL,
  `uuid` VARCHAR(16) NULL DEFAULT NULL,
  `last_login_timestamp` TIMESTAMP NULL DEFAULT NULL,
  `last_logout_timestamp` TIMESTAMP NULL DEFAULT NULL,
  `active` INT(11) NOT NULL DEFAULT '0',
  `language_id` INT(11) NULL DEFAULT NULL,
  `user_confirmation_date` TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`user_id`),
  UNIQUE INDEX `uuid_UNIQUE` (`uuid` ASC),
  INDEX `user_gamekid_id` (`user_gamekit_id` ASC),
  INDEX `user_gamekit_hash` (`user_gamekit_hash` ASC),
  INDEX `user_name` (`user_name` ASC),
  INDEX `user_name_match` (`user_name_match` ASC))
ENGINE = InnoDB
AUTO_INCREMENT = 16654
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`devices_android_agricola`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`devices_android_agricola` (
  `user_id` INT(10) UNSIGNED NOT NULL,
  `device_id` VARCHAR(1024) NULL DEFAULT '',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `devices_android_agricola_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`devices_android_summonwar`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`devices_android_summonwar` (
  `user_id` INT(10) UNSIGNED NOT NULL,
  `device_id` VARCHAR(1024) NULL DEFAULT '',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `devices_android_summonwar_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`devices_ios`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`devices_ios` (
  `user_id` INT(10) UNSIGNED NOT NULL,
  `device_id` TINYBLOB NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`),
  CONSTRAINT `devices_ios_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`devices_ios_agricola`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`devices_ios_agricola` (
  `user_id` INT(10) UNSIGNED NOT NULL,
  `device_id` TINYBLOB NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`),
  CONSTRAINT `devices_ios_agricola_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`devices_ios_fluxx`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`devices_ios_fluxx` (
  `user_id` INT(10) UNSIGNED NOT NULL,
  `device_id` TINYBLOB NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`),
  CONSTRAINT `devices_ios_fluxx_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`devices_ios_foodfight`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`devices_ios_foodfight` (
  `user_id` INT(10) UNSIGNED NOT NULL,
  `device_id` TINYBLOB NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`),
  CONSTRAINT `devices_ios_foodfight_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`devices_ios_nightfall`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`devices_ios_nightfall` (
  `user_id` INT(10) UNSIGNED NOT NULL,
  `device_id` TINYBLOB NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`),
  CONSTRAINT `devices_ios_nightfall_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`devices_ios_pennyarcade`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`devices_ios_pennyarcade` (
  `user_id` INT(10) UNSIGNED NOT NULL,
  `device_id` TINYBLOB NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`),
  CONSTRAINT `devices_ios_pennyarcade_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`devices_ios_smashup`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`devices_ios_smashup` (
  `user_id` INT(10) UNSIGNED NOT NULL,
  `device_id` TINYBLOB NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`),
  CONSTRAINT `devices_ios_smashup_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`devices_ios_summonwar`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`devices_ios_summonwar` (
  `user_id` INT(10) UNSIGNED NOT NULL,
  `device_id` TINYBLOB NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`),
  CONSTRAINT `devices_ios_summonwar_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`devices_ios_tantocuore`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`devices_ios_tantocuore` (
  `user_id` INT(10) UNSIGNED NOT NULL,
  `device_id` TINYBLOB NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`),
  CONSTRAINT `devices_ios_tantocuore_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`devices_ios_unsungcard`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`devices_ios_unsungcard` (
  `user_id` INT(10) UNSIGNED NOT NULL,
  `device_id` TINYBLOB NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`),
  CONSTRAINT `devices_ios_unsungcard_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`devices_ios_waterdeep`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`devices_ios_waterdeep` (
  `user_id` INT(10) UNSIGNED NOT NULL,
  `device_id` TINYBLOB NULL DEFAULT NULL,
  PRIMARY KEY (`user_id`),
  CONSTRAINT `devices_ios_waterdeep_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`friend_pending`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`friend_pending` (
  `id` INT(4) UNSIGNED ZEROFILL NOT NULL AUTO_INCREMENT,
  `inviter_id` INT(11) NULL DEFAULT NULL,
  `invitee_id` INT(11) NULL DEFAULT NULL,
  `was_notified` INT(11) NULL DEFAULT '0',
  `sent_date` TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP,
  `message` VARCHAR(64) NULL DEFAULT NULL,
  `uuid` VARCHAR(16) NULL DEFAULT NULL,
  PRIMARY KEY (`id`))
ENGINE = MyISAM
AUTO_INCREMENT = 344
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`friends`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`friends` (
  `userid1` INT(11) NULL DEFAULT '0',
  `userid2` INT(11) NULL DEFAULT '0',
  `id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`id`))
ENGINE = InnoDB
AUTO_INCREMENT = 386
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`friends_temp`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`friends_temp` (
  `user_id` INT(11) UNSIGNED NOT NULL,
  `friend_id` INT(11) UNSIGNED NOT NULL,
  INDEX `main` (`user_id` ASC))
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`games_agricola`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`games_agricola` (
  `game_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_version` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `max_players` INT(11) UNSIGNED NOT NULL DEFAULT '2',
  `random_seed` INT(11) UNSIGNED NOT NULL,
  `game_state` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `game_params` TINYBLOB NULL DEFAULT NULL,
  `rematch_id` INT(11) NOT NULL DEFAULT '0',
  `player_timer` INT(11) NOT NULL DEFAULT '0',
  `creation_time` DATETIME NOT NULL,
  `start_time` DATETIME NULL DEFAULT NULL,
  `completion_time` DATETIME NULL DEFAULT NULL,
  PRIMARY KEY (`game_id`))
ENGINE = InnoDB
AUTO_INCREMENT = 1738
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`games_ascension`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`games_ascension` (
  `game_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_version` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `max_players` INT(11) UNSIGNED NOT NULL DEFAULT '2',
  `random_seed` INT(11) UNSIGNED NOT NULL,
  `game_state` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `game_params` TINYBLOB NULL DEFAULT NULL,
  `rematch_id` INT(11) NOT NULL DEFAULT '0',
  `player_timer` INT(11) NOT NULL DEFAULT '0',
  `creation_time` DATETIME NOT NULL,
  `decision_time` DATETIME NULL DEFAULT NULL,
  `start_time` DATETIME NULL DEFAULT NULL,
  `completion_time` DATETIME NULL DEFAULT NULL,
  PRIMARY KEY (`game_id`))
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`games_fluxx`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`games_fluxx` (
  `game_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_version` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `max_players` INT(11) UNSIGNED NOT NULL DEFAULT '2',
  `random_seed` INT(11) UNSIGNED NOT NULL,
  `game_state` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `game_params` TINYBLOB NULL DEFAULT NULL,
  `rematch_id` INT(11) NOT NULL DEFAULT '0',
  `player_timer` INT(11) NOT NULL DEFAULT '0',
  `creation_time` DATETIME NOT NULL,
  `start_time` DATETIME NULL DEFAULT NULL,
  `completion_time` DATETIME NULL DEFAULT NULL,
  PRIMARY KEY (`game_id`))
ENGINE = InnoDB
AUTO_INCREMENT = 952
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`games_foodfight`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`games_foodfight` (
  `game_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_version` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `max_players` INT(11) UNSIGNED NOT NULL DEFAULT '2',
  `random_seed` INT(11) UNSIGNED NOT NULL,
  `game_state` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `game_params` TINYBLOB NULL DEFAULT NULL,
  `rematch_id` INT(11) NOT NULL DEFAULT '0',
  `player_timer` INT(11) NOT NULL DEFAULT '0',
  `creation_time` DATETIME NOT NULL,
  `decision_time` DATETIME NOT NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`game_id`))
ENGINE = InnoDB
AUTO_INCREMENT = 51
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`games_nightfall`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`games_nightfall` (
  `game_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_version` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `max_players` INT(11) UNSIGNED NOT NULL DEFAULT '2',
  `random_seed` INT(11) UNSIGNED NOT NULL,
  `game_state` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `game_params` TINYBLOB NULL DEFAULT NULL,
  `rematch_id` INT(11) NOT NULL DEFAULT '0',
  `player_timer` INT(11) NOT NULL DEFAULT '0',
  `creation_time` DATETIME NOT NULL,
  PRIMARY KEY (`game_id`))
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`games_pennyarcade`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`games_pennyarcade` (
  `game_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_version` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `max_players` INT(11) UNSIGNED NOT NULL DEFAULT '2',
  `random_seed` INT(11) UNSIGNED NOT NULL,
  `game_state` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `game_params` TINYBLOB NULL DEFAULT NULL,
  `rematch_id` INT(11) NOT NULL DEFAULT '0',
  `player_timer` INT(11) NOT NULL DEFAULT '0',
  `creation_time` DATETIME NOT NULL,
  `start_time` DATETIME NULL DEFAULT NULL,
  `completion_time` DATETIME NULL DEFAULT NULL,
  PRIMARY KEY (`game_id`))
ENGINE = InnoDB
AUTO_INCREMENT = 910
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`games_smashup`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`games_smashup` (
  `game_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_version` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `max_players` INT(11) UNSIGNED NOT NULL DEFAULT '2',
  `random_seed` INT(11) UNSIGNED NOT NULL,
  `game_state` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `game_params` TINYBLOB NULL DEFAULT NULL,
  `rematch_id` INT(11) NOT NULL DEFAULT '0',
  `player_timer` INT(11) NOT NULL DEFAULT '0',
  `creation_time` DATETIME NOT NULL,
  PRIMARY KEY (`game_id`))
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`games_summonwar`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`games_summonwar` (
  `game_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_version` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `max_players` INT(11) UNSIGNED NOT NULL DEFAULT '2',
  `random_seed` INT(11) UNSIGNED NOT NULL,
  `game_state` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `game_params` TINYBLOB NULL DEFAULT NULL,
  `rematch_id` INT(11) NOT NULL DEFAULT '0',
  `player_timer` INT(11) NOT NULL DEFAULT '0',
  `event_id` INT(10) UNSIGNED NULL DEFAULT NULL,
  `event_matchup_index` INT(10) UNSIGNED NULL DEFAULT NULL,
  `creation_time` DATETIME NOT NULL,
  `decision_time` DATETIME NOT NULL DEFAULT '0000-00-00 00:00:00',
  `start_time` DATETIME NULL DEFAULT NULL,
  `completion_time` DATETIME NULL DEFAULT NULL,
  PRIMARY KEY (`game_id`))
ENGINE = InnoDB
AUTO_INCREMENT = 234
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`games_summonwar_2`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`games_summonwar_2` (
  `game_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_version` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `max_players` INT(11) UNSIGNED NOT NULL DEFAULT '2',
  `random_seed` INT(11) UNSIGNED NOT NULL,
  `game_state` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `game_params` TINYBLOB NULL DEFAULT NULL,
  `rematch_id` INT(11) NOT NULL DEFAULT '0',
  `player_timer` INT(11) NOT NULL DEFAULT '0',
  `event_id` INT(10) UNSIGNED NULL DEFAULT NULL,
  `event_matchup_index` INT(10) UNSIGNED NULL DEFAULT NULL,
  `creation_time` DATETIME NOT NULL,
  `decision_time` DATETIME NOT NULL DEFAULT '0000-00-00 00:00:00',
  `start_time` DATETIME NULL DEFAULT NULL,
  `completion_time` DATETIME NULL DEFAULT NULL,
  PRIMARY KEY (`game_id`))
ENGINE = InnoDB
AUTO_INCREMENT = 234
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`games_tantocuore`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`games_tantocuore` (
  `game_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_version` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `max_players` INT(11) UNSIGNED NOT NULL DEFAULT '2',
  `random_seed` INT(11) UNSIGNED NOT NULL,
  `game_state` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `game_params` TINYBLOB NULL DEFAULT NULL,
  `rematch_id` INT(11) NOT NULL DEFAULT '0',
  `player_timer` INT(11) NOT NULL DEFAULT '0',
  `creation_time` DATETIME NOT NULL,
  `start_time` DATETIME NULL DEFAULT NULL,
  `completion_time` DATETIME NULL DEFAULT NULL,
  PRIMARY KEY (`game_id`))
ENGINE = InnoDB
AUTO_INCREMENT = 463
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`games_unsungcard`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`games_unsungcard` (
  `game_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_version` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `max_players` INT(11) UNSIGNED NOT NULL DEFAULT '2',
  `random_seed` INT(11) UNSIGNED NOT NULL,
  `game_state` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `game_params` TINYBLOB NULL DEFAULT NULL,
  `rematch_id` INT(11) NOT NULL DEFAULT '0',
  `player_timer` INT(11) NOT NULL DEFAULT '0',
  `creation_time` DATETIME NOT NULL,
  `start_time` DATETIME NULL DEFAULT NULL,
  `completion_time` DATETIME NULL DEFAULT NULL,
  PRIMARY KEY (`game_id`))
ENGINE = InnoDB
AUTO_INCREMENT = 14
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`games_waterdeep`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`games_waterdeep` (
  `game_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_version` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `max_players` INT(11) UNSIGNED NOT NULL DEFAULT '2',
  `random_seed` INT(11) UNSIGNED NOT NULL,
  `game_state` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `game_params` TINYBLOB NULL DEFAULT NULL,
  `rematch_id` INT(11) NOT NULL DEFAULT '0',
  `player_timer` INT(11) NOT NULL DEFAULT '0',
  `creation_time` DATETIME NOT NULL,
  `start_time` DATETIME NULL DEFAULT NULL,
  `completion_time` DATETIME NULL DEFAULT NULL,
  PRIMARY KEY (`game_id`))
ENGINE = InnoDB
AUTO_INCREMENT = 981
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`invalid_username`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`invalid_username` (
  `id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `user_name_match` VARCHAR(32) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`))
ENGINE = InnoDB
AUTO_INCREMENT = 27
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`invitation`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`invitation` (
  `id` INT(4) UNSIGNED ZEROFILL NOT NULL AUTO_INCREMENT,
  `inviter_id` VARCHAR(16) NULL DEFAULT NULL,
  `invitee_id` VARCHAR(16) NULL DEFAULT NULL,
  `group_uuid` VARCHAR(16) NULL DEFAULT NULL,
  `date` TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP,
  `message` VARCHAR(64) NULL DEFAULT NULL,
  `uuid` VARCHAR(16) NULL DEFAULT NULL,
  `type` INT(11) NULL DEFAULT '0',
  PRIMARY KEY (`id`))
ENGINE = MyISAM
AUTO_INCREMENT = 386
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`language`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`language` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `language_name` VARCHAR(45) NULL DEFAULT NULL,
  `two_letter_code` VARCHAR(2) NULL DEFAULT NULL,
  PRIMARY KEY (`id`))
ENGINE = MyISAM
AUTO_INCREMENT = 10
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`log`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`log` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `text` VARCHAR(45) NOT NULL,
  `priority` INT(11) NULL DEFAULT '1',
  `time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `server_id` VARCHAR(45) NULL DEFAULT NULL,
  PRIMARY KEY (`id`))
ENGINE = MyISAM
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`matchmaking_agricola`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`matchmaking_agricola` (
  `match_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `user_id` INT(10) UNSIGNED NOT NULL,
  `max_players` INT(11) UNSIGNED NOT NULL DEFAULT '2',
  `player_timer` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `player_rating` INT(10) UNSIGNED NOT NULL DEFAULT '1500',
  `max_rating_range` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `max_wait_time` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `request_time` DATETIME NULL DEFAULT '0000-00-00 00:00:00',
  `game_params` TINYBLOB NULL DEFAULT NULL,
  `player_params` TINYBLOB NULL DEFAULT NULL,
  `created_game_id` INT(10) UNSIGNED NULL DEFAULT NULL,
  PRIMARY KEY (`match_id`),
  INDEX `user_id` (`user_id` ASC),
  INDEX `created_game_id` (`created_game_id` ASC))
ENGINE = InnoDB
AUTO_INCREMENT = 8
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`matchmaking_unsungcard`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`matchmaking_unsungcard` (
  `match_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `user_id` INT(10) UNSIGNED NOT NULL,
  `max_players` INT(11) UNSIGNED NOT NULL DEFAULT '2',
  `player_timer` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `player_rating` INT(10) UNSIGNED NOT NULL DEFAULT '1500',
  `max_rating_range` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `max_wait_time` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `request_time` DATETIME NULL DEFAULT '0000-00-00 00:00:00',
  `game_params` TINYBLOB NULL DEFAULT NULL,
  `player_params` TINYBLOB NULL DEFAULT NULL,
  `created_game_id` INT(10) UNSIGNED NULL DEFAULT NULL,
  PRIMARY KEY (`match_id`),
  INDEX `user_id` (`user_id` ASC),
  INDEX `created_game_id` (`created_game_id` ASC))
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`matchmaking_waterdeep`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`matchmaking_waterdeep` (
  `match_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `user_id` INT(10) UNSIGNED NOT NULL,
  `max_players` INT(11) UNSIGNED NOT NULL DEFAULT '2',
  `player_timer` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `player_rating` INT(10) UNSIGNED NOT NULL DEFAULT '1500',
  `max_rating_range` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `max_wait_time` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `request_time` DATETIME NULL DEFAULT '0000-00-00 00:00:00',
  `game_params` TINYBLOB NULL DEFAULT NULL,
  `player_params` TINYBLOB NULL DEFAULT NULL,
  `created_game_id` INT(10) UNSIGNED NULL DEFAULT NULL,
  PRIMARY KEY (`match_id`),
  INDEX `user_id` (`user_id` ASC),
  INDEX `created_game_id` (`created_game_id` ASC))
ENGINE = InnoDB
AUTO_INCREMENT = 228
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`moves_agricola`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`moves_agricola` (
  `move_id` INT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(11) UNSIGNED NOT NULL,
  `user_id` INT(11) UNSIGNED NOT NULL,
  `selection_idx` INT(11) NOT NULL,
  `selection_id` INT(11) UNSIGNED NOT NULL,
  `selection_hint` INT(11) UNSIGNED NOT NULL,
  `selection_data` INT(11) UNSIGNED NULL DEFAULT NULL,
  PRIMARY KEY (`move_id`),
  INDEX `game_id` (`game_id` ASC),
  INDEX `user_id` (`user_id` ASC),
  CONSTRAINT `moves_agricola_ibfk_1`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_agricola` (`game_id`)
    ON DELETE CASCADE,
  CONSTRAINT `moves_agricola_ibfk_2`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE NO ACTION)
ENGINE = InnoDB
AUTO_INCREMENT = 160985
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`moves_ascension`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`moves_ascension` (
  `move_id` INT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(11) UNSIGNED NOT NULL,
  `user_id` INT(11) UNSIGNED NOT NULL,
  `selection_idx` INT(11) NOT NULL,
  `selection_id` INT(11) UNSIGNED NOT NULL,
  `selection_hint` INT(11) UNSIGNED NOT NULL,
  PRIMARY KEY (`move_id`),
  INDEX `game_id` (`game_id` ASC),
  INDEX `user_id` (`user_id` ASC),
  CONSTRAINT `moves_ascension_ibfk_1`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_ascension` (`game_id`)
    ON DELETE CASCADE,
  CONSTRAINT `moves_ascension_ibfk_2`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`moves_fluxx`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`moves_fluxx` (
  `move_id` INT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(11) UNSIGNED NOT NULL,
  `user_id` INT(11) UNSIGNED NOT NULL,
  `selection_idx` INT(11) NOT NULL,
  `selection_id` INT(11) UNSIGNED NOT NULL,
  `selection_hint` INT(11) UNSIGNED NOT NULL,
  PRIMARY KEY (`move_id`),
  INDEX `game_id` (`game_id` ASC),
  INDEX `user_id` (`user_id` ASC),
  CONSTRAINT `moves_fluxx_ibfk_1`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_fluxx` (`game_id`)
    ON DELETE CASCADE,
  CONSTRAINT `moves_fluxx_ibfk_2`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE NO ACTION)
ENGINE = InnoDB
AUTO_INCREMENT = 19730
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`moves_foodfight`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`moves_foodfight` (
  `move_id` INT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(11) UNSIGNED NOT NULL,
  `user_id` INT(11) UNSIGNED NOT NULL,
  `selection_idx` INT(11) NOT NULL,
  `selection_id` INT(11) UNSIGNED NOT NULL,
  `selection_hint` INT(11) UNSIGNED NOT NULL,
  PRIMARY KEY (`move_id`),
  INDEX `game_id` (`game_id` ASC),
  INDEX `user_id` (`user_id` ASC),
  CONSTRAINT `moves_foodfight_ibfk_1`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_foodfight` (`game_id`)
    ON DELETE CASCADE,
  CONSTRAINT `moves_foodfight_ibfk_2`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE NO ACTION)
ENGINE = InnoDB
AUTO_INCREMENT = 15413
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`moves_nightfall`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`moves_nightfall` (
  `move_id` INT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(11) UNSIGNED NOT NULL,
  `user_id` INT(11) UNSIGNED NOT NULL,
  `selection_idx` INT(11) NOT NULL,
  `selection_id` INT(11) UNSIGNED NOT NULL,
  `selection_hint` INT(11) UNSIGNED NOT NULL,
  PRIMARY KEY (`move_id`),
  INDEX `game_id` (`game_id` ASC),
  INDEX `user_id` (`user_id` ASC),
  CONSTRAINT `moves_nightfall_ibfk_1`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_nightfall` (`game_id`)
    ON DELETE CASCADE,
  CONSTRAINT `moves_nightfall_ibfk_2`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`moves_pennyarcade`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`moves_pennyarcade` (
  `move_id` INT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(11) UNSIGNED NOT NULL,
  `user_id` INT(11) UNSIGNED NOT NULL,
  `selection_idx` INT(11) NOT NULL,
  `selection_id` INT(11) UNSIGNED NOT NULL,
  `selection_hint` INT(11) UNSIGNED NOT NULL,
  PRIMARY KEY (`move_id`),
  INDEX `game_id` (`game_id` ASC),
  INDEX `user_id` (`user_id` ASC),
  CONSTRAINT `moves_pennyarcade_ibfk_1`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_pennyarcade` (`game_id`)
    ON DELETE CASCADE,
  CONSTRAINT `moves_pennyarcade_ibfk_2`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE NO ACTION)
ENGINE = InnoDB
AUTO_INCREMENT = 48533
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`moves_smashup`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`moves_smashup` (
  `move_id` INT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(11) UNSIGNED NOT NULL,
  `user_id` INT(11) UNSIGNED NOT NULL,
  `selection_idx` INT(11) NOT NULL,
  `selection_id` INT(11) UNSIGNED NOT NULL,
  `selection_hint` INT(11) UNSIGNED NOT NULL,
  PRIMARY KEY (`move_id`),
  INDEX `game_id` (`game_id` ASC),
  INDEX `user_id` (`user_id` ASC),
  CONSTRAINT `moves_smashup_ibfk_1`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_smashup` (`game_id`)
    ON DELETE CASCADE,
  CONSTRAINT `moves_smashup_ibfk_2`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`moves_summonwar`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`moves_summonwar` (
  `move_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(11) UNSIGNED NOT NULL,
  `user_id` INT(11) UNSIGNED NOT NULL,
  `selection_idx` INT(11) NOT NULL,
  `selection_id` INT(11) UNSIGNED NOT NULL,
  `selection_hint` INT(11) UNSIGNED NOT NULL,
  `selection_data` INT(11) UNSIGNED NULL DEFAULT NULL,
  PRIMARY KEY (`move_id`),
  INDEX `user_id` (`user_id` ASC),
  INDEX `game_id` (`game_id` ASC),
  CONSTRAINT `moves_summonwar_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
  CONSTRAINT `moves_summonwar_ibfk_2`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_summonwar` (`game_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
AUTO_INCREMENT = 11164
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`moves_tantocuore`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`moves_tantocuore` (
  `move_id` INT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(11) UNSIGNED NOT NULL,
  `user_id` INT(11) UNSIGNED NOT NULL,
  `selection_idx` INT(11) NOT NULL,
  `selection_id` INT(11) UNSIGNED NOT NULL,
  `selection_hint` INT(11) UNSIGNED NOT NULL,
  PRIMARY KEY (`move_id`),
  INDEX `game_id` (`game_id` ASC),
  INDEX `user_id` (`user_id` ASC),
  CONSTRAINT `moves_tantocuore_ibfk_1`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_tantocuore` (`game_id`)
    ON DELETE CASCADE,
  CONSTRAINT `moves_tantocuore_ibfk_2`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE NO ACTION)
ENGINE = InnoDB
AUTO_INCREMENT = 30849
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`moves_unsungcard`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`moves_unsungcard` (
  `move_id` INT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(11) UNSIGNED NOT NULL,
  `user_id` INT(11) UNSIGNED NOT NULL,
  `selection_idx` INT(11) NOT NULL,
  `selection_id` INT(11) UNSIGNED NOT NULL,
  `selection_hint` INT(11) UNSIGNED NOT NULL,
  `selection_data` INT(11) UNSIGNED NULL DEFAULT NULL,
  PRIMARY KEY (`move_id`),
  INDEX `game_id` (`game_id` ASC),
  INDEX `user_id` (`user_id` ASC),
  CONSTRAINT `moves_unsungcard_ibfk_1`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_unsungcard` (`game_id`)
    ON DELETE CASCADE,
  CONSTRAINT `moves_unsungcard_ibfk_2`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`moves_waterdeep`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`moves_waterdeep` (
  `move_id` INT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(11) UNSIGNED NOT NULL,
  `user_id` INT(11) UNSIGNED NOT NULL,
  `selection_idx` INT(11) NOT NULL,
  `selection_id` INT(11) UNSIGNED NOT NULL,
  `selection_hint` INT(11) UNSIGNED NOT NULL,
  `selection_data` INT(11) UNSIGNED NULL DEFAULT NULL,
  PRIMARY KEY (`move_id`),
  INDEX `game_id` (`game_id` ASC),
  INDEX `user_id` (`user_id` ASC),
  CONSTRAINT `moves_waterdeep_ibfk_1`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_waterdeep` (`game_id`)
    ON DELETE CASCADE,
  CONSTRAINT `moves_waterdeep_ibfk_2`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE NO ACTION)
ENGINE = InnoDB
AUTO_INCREMENT = 58107
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`players_agricola`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`players_agricola` (
  `player_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(10) UNSIGNED NOT NULL,
  `user_id` INT(10) UNSIGNED NOT NULL,
  `player_state` INT(11) NOT NULL,
  `player_slot` INT(10) UNSIGNED NOT NULL,
  `player_position` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `player_timer` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `decision_time` DATETIME NULL DEFAULT '0000-00-00 00:00:00',
  `player_params` TINYBLOB NULL DEFAULT NULL,
  PRIMARY KEY (`player_id`),
  INDEX `user_id` (`user_id` ASC),
  INDEX `game_id` (`game_id` ASC),
  CONSTRAINT `players_agricola_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
  CONSTRAINT `players_agricola_ibfk_2`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_agricola` (`game_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
AUTO_INCREMENT = 3986
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`players_ascension`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`players_ascension` (
  `player_id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(10) UNSIGNED NOT NULL,
  `user_id` INT(10) UNSIGNED NOT NULL,
  `player_state` INT(11) NOT NULL,
  `player_slot` INT(10) UNSIGNED NOT NULL,
  `player_position` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `player_timer` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`player_id`),
  INDEX `user_id` (`user_id` ASC),
  INDEX `game_id` (`game_id` ASC),
  CONSTRAINT `game_id`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_ascension` (`game_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
  CONSTRAINT `user_id`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`players_fluxx`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`players_fluxx` (
  `player_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(10) UNSIGNED NOT NULL,
  `user_id` INT(10) UNSIGNED NOT NULL,
  `player_state` INT(11) NOT NULL,
  `player_slot` INT(10) UNSIGNED NOT NULL,
  `player_position` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `player_timer` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `decision_time` DATETIME NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`player_id`),
  INDEX `user_id` (`user_id` ASC),
  INDEX `game_id` (`game_id` ASC),
  CONSTRAINT `players_fluxx_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
  CONSTRAINT `players_fluxx_ibfk_2`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_fluxx` (`game_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
AUTO_INCREMENT = 1817
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`players_foodfight`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`players_foodfight` (
  `player_id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(10) UNSIGNED NOT NULL,
  `user_id` INT(10) UNSIGNED NOT NULL,
  `player_state` INT(11) NOT NULL,
  `player_slot` INT(10) UNSIGNED NOT NULL,
  `player_position` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `player_timer` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`player_id`),
  INDEX `user_id` (`user_id` ASC),
  INDEX `game_id` (`game_id` ASC),
  CONSTRAINT `players_foodfight_ibfk_1`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_foodfight` (`game_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
  CONSTRAINT `players_foodfight_ibfk_2`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
AUTO_INCREMENT = 526
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`players_nightfall`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`players_nightfall` (
  `player_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(10) UNSIGNED NOT NULL,
  `user_id` INT(10) UNSIGNED NOT NULL,
  `player_state` INT(11) NOT NULL,
  `player_slot` INT(10) UNSIGNED NOT NULL,
  `player_position` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `player_timer` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `decision_time` DATETIME NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`player_id`),
  INDEX `user_id` (`user_id` ASC),
  INDEX `game_id` (`game_id` ASC),
  CONSTRAINT `players_nightfall_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
  CONSTRAINT `players_nightfall_ibfk_2`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_nightfall` (`game_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`players_pennyarcade`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`players_pennyarcade` (
  `player_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(10) UNSIGNED NOT NULL,
  `user_id` INT(10) UNSIGNED NOT NULL,
  `player_state` INT(11) NOT NULL,
  `player_slot` INT(10) UNSIGNED NOT NULL,
  `player_position` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `player_timer` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `decision_time` DATETIME NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`player_id`),
  INDEX `user_id` (`user_id` ASC),
  INDEX `game_id` (`game_id` ASC),
  CONSTRAINT `players_pennyarcade_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
  CONSTRAINT `players_pennyarcade_ibfk_2`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_pennyarcade` (`game_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
AUTO_INCREMENT = 1697
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`players_smashup`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`players_smashup` (
  `player_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(10) UNSIGNED NOT NULL,
  `user_id` INT(10) UNSIGNED NOT NULL,
  `player_state` INT(11) NOT NULL,
  `player_slot` INT(10) UNSIGNED NOT NULL,
  `player_position` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `player_timer` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `decision_time` DATETIME NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`player_id`),
  INDEX `user_id` (`user_id` ASC),
  INDEX `game_id` (`game_id` ASC),
  CONSTRAINT `players_smashup_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
  CONSTRAINT `players_smashup_ibfk_2`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_smashup` (`game_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`players_summonwar`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`players_summonwar` (
  `player_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(10) UNSIGNED NOT NULL,
  `user_id` INT(10) UNSIGNED NOT NULL,
  `player_state` INT(11) NOT NULL,
  `player_slot` INT(10) UNSIGNED NOT NULL,
  `player_position` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `player_timer` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `player_params` TINYBLOB NULL DEFAULT NULL,
  PRIMARY KEY (`player_id`),
  INDEX `user_id` (`user_id` ASC),
  INDEX `game_id` (`game_id` ASC),
  CONSTRAINT `players_summonwar_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
  CONSTRAINT `players_summonwar_ibfk_2`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_summonwar` (`game_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
AUTO_INCREMENT = 439
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`players_summonwar_2`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`players_summonwar_2` (
  `player_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(10) UNSIGNED NOT NULL,
  `user_id` INT(10) UNSIGNED NOT NULL,
  `player_state` INT(11) NOT NULL,
  `player_slot` INT(10) UNSIGNED NOT NULL,
  `player_position` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `player_timer` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `player_params` TINYBLOB NULL DEFAULT NULL,
  PRIMARY KEY (`player_id`),
  INDEX `user_id` (`user_id` ASC),
  INDEX `game_id` (`game_id` ASC),
  CONSTRAINT `players_summonwar_2_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
  CONSTRAINT `players_summonwar_2_ibfk_2`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_summonwar` (`game_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
AUTO_INCREMENT = 439
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`players_tantocuore`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`players_tantocuore` (
  `player_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(10) UNSIGNED NOT NULL,
  `user_id` INT(10) UNSIGNED NOT NULL,
  `player_state` INT(11) NOT NULL,
  `player_slot` INT(10) UNSIGNED NOT NULL,
  `player_position` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `player_timer` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `decision_time` DATETIME NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`player_id`),
  INDEX `user_id` (`user_id` ASC),
  INDEX `game_id` (`game_id` ASC),
  CONSTRAINT `players_tantocuore_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
  CONSTRAINT `players_tantocuore_ibfk_2`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_tantocuore` (`game_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
AUTO_INCREMENT = 874
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`players_unsungcard`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`players_unsungcard` (
  `player_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(10) UNSIGNED NOT NULL,
  `user_id` INT(10) UNSIGNED NOT NULL,
  `player_state` INT(11) NOT NULL,
  `player_slot` INT(10) UNSIGNED NOT NULL,
  `player_position` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `player_timer` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `decision_time` DATETIME NULL DEFAULT '0000-00-00 00:00:00',
  `player_params` TINYBLOB NULL DEFAULT NULL,
  PRIMARY KEY (`player_id`),
  INDEX `user_id` (`user_id` ASC),
  INDEX `game_id` (`game_id` ASC),
  CONSTRAINT `players_unsungcard_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
  CONSTRAINT `players_unsungcard_ibfk_2`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_unsungcard` (`game_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
AUTO_INCREMENT = 16
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`players_waterdeep`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`players_waterdeep` (
  `player_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(10) UNSIGNED NOT NULL,
  `user_id` INT(10) UNSIGNED NOT NULL,
  `player_state` INT(11) NOT NULL,
  `player_slot` INT(10) UNSIGNED NOT NULL,
  `player_position` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `player_timer` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `decision_time` DATETIME NULL DEFAULT '0000-00-00 00:00:00',
  `player_params` TINYBLOB NULL DEFAULT NULL,
  PRIMARY KEY (`player_id`),
  INDEX `user_id` (`user_id` ASC),
  INDEX `game_id` (`game_id` ASC),
  CONSTRAINT `players_waterdeep_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
  CONSTRAINT `players_waterdeep_ibfk_2`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_waterdeep` (`game_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
AUTO_INCREMENT = 2207
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`precondition`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`precondition` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `precondition_group_id` INT(11) NULL DEFAULT NULL,
  `type` TINYINT(4) NULL DEFAULT NULL COMMENT '<, <=, ==, >=, >, !=, !, non-zero, exists in table',
  `field_to_compare` VARCHAR(45) NULL DEFAULT '',
  `reference_object` VARCHAR(45) NULL DEFAULT '' COMMENT 'player, user, game, some struct used for comparison.. could be a table',
  `value_to_compare` VARCHAR(45) NULL DEFAULT NULL,
  `name` VARCHAR(45) NULL DEFAULT NULL,
  `description_string_lookup` VARCHAR(45) NULL DEFAULT NULL,
  `notes` VARCHAR(45) NULL DEFAULT NULL,
  PRIMARY KEY (`id`))
ENGINE = MyISAM
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`precondition_group`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`precondition_group` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `type` VARCHAR(45) NULL DEFAULT '1' COMMENT 'and, or, xor',
  `parent_group_id` VARCHAR(45) NULL DEFAULT '0' COMMENT 'recursion',
  `name` VARCHAR(45) NULL DEFAULT NULL,
  `description_string_lookup` VARCHAR(45) NULL DEFAULT NULL,
  `notes` VARCHAR(45) NULL DEFAULT NULL,
  PRIMARY KEY (`id`))
ENGINE = MyISAM
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`product`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`product` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `product_id` INT(11) NULL DEFAULT NULL,
  `uuid` VARCHAR(16) NULL DEFAULT NULL,
  `name` VARCHAR(45) NULL DEFAULT NULL,
  `filter_name` VARCHAR(45) NULL DEFAULT NULL,
  `first_available` TIMESTAMP NULL DEFAULT NULL,
  `product_type` INT(11) NULL DEFAULT NULL,
  `notes` VARCHAR(255) NULL DEFAULT NULL,
  `name_string` VARCHAR(45) NULL DEFAULT '0',
  `icon_lookup` VARCHAR(45) NULL DEFAULT '0',
  PRIMARY KEY (`id`))
ENGINE = MyISAM
AUTO_INCREMENT = 3010
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`product_exchange_rate`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`product_exchange_rate` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `product_source_id` INT(11) NULL DEFAULT NULL,
  `product_dest_id` INT(11) NULL DEFAULT NULL,
  `source_count` INT(11) NULL DEFAULT '1',
  `dest_count` INT(11) NULL DEFAULT '1',
  `begin_date` DATETIME NULL DEFAULT NULL,
  `end_date` DATETIME NULL DEFAULT NULL,
  `notes` VARCHAR(255) NULL DEFAULT NULL,
  `created_by` VARCHAR(45) NULL DEFAULT NULL,
  `enchange_uuid` VARCHAR(45) NULL DEFAULT NULL,
  `title_string` VARCHAR(45) NULL DEFAULT '0',
  `description_string` VARCHAR(45) NULL DEFAULT '0',
  `custom_uuid` VARCHAR(16) NULL DEFAULT '0' COMMENT 'For non-product items like tournament entry, store the tournament uuid here',
  PRIMARY KEY (`id`))
ENGINE = MyISAM
AUTO_INCREMENT = 6
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`product_type`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`product_type` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `description` VARCHAR(45) NULL DEFAULT NULL,
  `notes` VARCHAR(255) NULL DEFAULT NULL,
  PRIMARY KEY (`id`))
ENGINE = MyISAM
AUTO_INCREMENT = 7
DEFAULT CHARACTER SET = utf8
COMMENT = 'Just a lookup table	';


-- -----------------------------------------------------
-- Table `playdek`.`reset_password_keys`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`reset_password_keys` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `user_account_uuid` VARCHAR(16) NULL DEFAULT NULL,
  `reset_key` VARCHAR(32) NULL DEFAULT NULL,
  `possible_password` VARCHAR(45) NULL DEFAULT NULL,
  `was_email_sent` INT(1) NULL DEFAULT '0',
  `time_email_last_sent` TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`))
ENGINE = MyISAM
AUTO_INCREMENT = 78
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `playdek`.`reset_user_email_name`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`reset_user_email_name` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `user_account_uuid` VARCHAR(16) NULL DEFAULT NULL,
  `reset_key` VARCHAR(45) NULL DEFAULT NULL,
  `new_email` VARCHAR(45) NULL DEFAULT NULL,
  `new_username` VARCHAR(45) NULL DEFAULT NULL,
  `was_email_sent` TINYINT(4) NULL DEFAULT '0',
  `time_last_email_sent` TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE INDEX `user_account_uuid_UNIQUE` (`user_account_uuid` ASC))
ENGINE = MyISAM
AUTO_INCREMENT = 7
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`results_agricola`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`results_agricola` (
  `result_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(10) UNSIGNED NOT NULL,
  `user_id` INT(10) UNSIGNED NOT NULL,
  `finish_position` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `initial_rating` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `updated_rating` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`result_id`),
  INDEX `user_id` (`user_id` ASC),
  INDEX `game_id` (`game_id` ASC),
  CONSTRAINT `results_agricola_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
  CONSTRAINT `results_agricola_ibfk_2`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_agricola` (`game_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
AUTO_INCREMENT = 598
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`results_unsungcard`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`results_unsungcard` (
  `result_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(10) UNSIGNED NOT NULL,
  `user_id` INT(10) UNSIGNED NOT NULL,
  `finish_position` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `initial_rating` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `updated_rating` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`result_id`),
  INDEX `user_id` (`user_id` ASC),
  INDEX `game_id` (`game_id` ASC),
  CONSTRAINT `results_unsungcard_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
  CONSTRAINT `results_unsungcard_ibfk_2`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_unsungcard` (`game_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`results_waterdeep`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`results_waterdeep` (
  `result_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `game_id` INT(10) UNSIGNED NOT NULL,
  `user_id` INT(10) UNSIGNED NOT NULL,
  `finish_position` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `initial_rating` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `updated_rating` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`result_id`),
  INDEX `user_id` (`user_id` ASC),
  INDEX `game_id` (`game_id` ASC),
  CONSTRAINT `results_waterdeep_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
  CONSTRAINT `results_waterdeep_ibfk_2`
    FOREIGN KEY (`game_id`)
    REFERENCES `playdek`.`games_waterdeep` (`game_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
AUTO_INCREMENT = 570
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`stats_agricola`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`stats_agricola` (
  `user_id` INT(10) UNSIGNED NOT NULL,
  `rating` FLOAT NOT NULL,
  `deviation` FLOAT NOT NULL,
  `volatility` FLOAT NOT NULL,
  `completed_games` INT(10) NOT NULL DEFAULT '0',
  `wins_2p` INT(10) NOT NULL DEFAULT '0',
  `losses_2p` INT(10) NOT NULL DEFAULT '0',
  `wins_3p` INT(10) NOT NULL DEFAULT '0',
  `losses_3p` INT(10) NOT NULL DEFAULT '0',
  `wins_4p` INT(10) NOT NULL DEFAULT '0',
  `losses_4p` INT(10) NOT NULL DEFAULT '0',
  `wins_5p` INT(10) NOT NULL DEFAULT '0',
  `losses_5p` INT(10) NOT NULL DEFAULT '0',
  `forfeits` INT(10) NOT NULL DEFAULT '0',
  `wins_by_forfeit` INT(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `stats_agricola_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`stats_ascension`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`stats_ascension` (
  `user_id` INT(10) UNSIGNED NOT NULL,
  `completed_games` INT(10) NOT NULL DEFAULT '0',
  `wins_2p` INT(10) NOT NULL DEFAULT '0',
  `losses_2p` INT(10) NOT NULL DEFAULT '0',
  `wins_3p` INT(10) NOT NULL DEFAULT '0',
  `losses_3p` INT(10) NOT NULL DEFAULT '0',
  `wins_4p` INT(10) NOT NULL DEFAULT '0',
  `losses_4p` INT(10) NOT NULL DEFAULT '0',
  `forfeits` INT(10) NOT NULL DEFAULT '0',
  `wins_by_forfeit` INT(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `stats_ascension_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`stats_fluxx`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`stats_fluxx` (
  `user_id` INT(10) UNSIGNED NOT NULL,
  `completed_games` INT(10) NOT NULL DEFAULT '0',
  `wins_2p` INT(10) NOT NULL DEFAULT '0',
  `losses_2p` INT(10) NOT NULL DEFAULT '0',
  `wins_3p` INT(10) NOT NULL DEFAULT '0',
  `losses_3p` INT(10) NOT NULL DEFAULT '0',
  `wins_4p` INT(10) NOT NULL DEFAULT '0',
  `losses_4p` INT(10) NOT NULL DEFAULT '0',
  `forfeits` INT(10) NOT NULL DEFAULT '0',
  `wins_by_forfeit` INT(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `stats_fluxx_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`stats_foodfight`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`stats_foodfight` (
  `user_id` INT(10) UNSIGNED NOT NULL,
  `completed_games` INT(10) NOT NULL DEFAULT '0',
  `wins_2p` INT(10) NOT NULL DEFAULT '0',
  `losses_2p` INT(10) NOT NULL DEFAULT '0',
  `wins_3p` INT(10) NOT NULL DEFAULT '0',
  `losses_3p` INT(10) NOT NULL DEFAULT '0',
  `wins_4p` INT(10) NOT NULL DEFAULT '0',
  `losses_4p` INT(10) NOT NULL DEFAULT '0',
  `forfeits` INT(10) NOT NULL DEFAULT '0',
  `wins_by_forfeit` INT(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `stats_foodfight_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`stats_nightfall`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`stats_nightfall` (
  `user_id` INT(10) UNSIGNED NOT NULL,
  `completed_games` INT(10) NOT NULL DEFAULT '0',
  `wins_2p` INT(10) NOT NULL DEFAULT '0',
  `losses_2p` INT(10) NOT NULL DEFAULT '0',
  `wins_3p` INT(10) NOT NULL DEFAULT '0',
  `losses_3p` INT(10) NOT NULL DEFAULT '0',
  `wins_4p` INT(10) NOT NULL DEFAULT '0',
  `losses_4p` INT(10) NOT NULL DEFAULT '0',
  `wins_5p` INT(10) NOT NULL DEFAULT '0',
  `losses_5p` INT(10) NOT NULL DEFAULT '0',
  `forfeits` INT(10) NOT NULL DEFAULT '0',
  `wins_by_forfeit` INT(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `stats_nightfall_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`stats_pennyarcade`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`stats_pennyarcade` (
  `user_id` INT(10) UNSIGNED NOT NULL,
  `completed_games` INT(10) NOT NULL DEFAULT '0',
  `wins_2p` INT(10) NOT NULL DEFAULT '0',
  `losses_2p` INT(10) NOT NULL DEFAULT '0',
  `wins_3p` INT(10) NOT NULL DEFAULT '0',
  `losses_3p` INT(10) NOT NULL DEFAULT '0',
  `wins_4p` INT(10) NOT NULL DEFAULT '0',
  `losses_4p` INT(10) NOT NULL DEFAULT '0',
  `forfeits` INT(10) NOT NULL DEFAULT '0',
  `wins_by_forfeit` INT(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `stats_pennyarcade_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`stats_server`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`stats_server` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `stat_name` VARCHAR(60) NULL DEFAULT NULL,
  `server_reporting` VARCHAR(45) NULL DEFAULT 'gateway',
  `category` INT(2) NULL DEFAULT NULL,
  `sub_category` INT(2) NULL DEFAULT NULL,
  `mean` FLOAT NULL DEFAULT '0',
  `final_value` FLOAT NULL DEFAULT '0',
  `min_value` FLOAT NULL DEFAULT '0',
  `max_value` FLOAT NULL DEFAULT '0',
  `num_values` INT(11) NULL DEFAULT '1' COMMENT 'How many entries',
  `std_dev` FLOAT NULL DEFAULT '0',
  `begin_time` TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP,
  `end_time` DATETIME NULL DEFAULT NULL,
  PRIMARY KEY (`id`))
ENGINE = MyISAM
AUTO_INCREMENT = 18591
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`stats_smashup`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`stats_smashup` (
  `user_id` INT(10) UNSIGNED NOT NULL,
  `completed_games` INT(10) NOT NULL DEFAULT '0',
  `wins_2p` INT(10) NOT NULL DEFAULT '0',
  `losses_2p` INT(10) NOT NULL DEFAULT '0',
  `wins_3p` INT(10) NOT NULL DEFAULT '0',
  `losses_3p` INT(10) NOT NULL DEFAULT '0',
  `wins_4p` INT(10) NOT NULL DEFAULT '0',
  `losses_4p` INT(10) NOT NULL DEFAULT '0',
  `forfeits` INT(10) NOT NULL DEFAULT '0',
  `wins_by_forfeit` INT(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `stats_smashup_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`stats_summonwar`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`stats_summonwar` (
  `user_id` INT(10) UNSIGNED NOT NULL,
  `completed_games` INT(10) NOT NULL DEFAULT '0',
  `wins_2p` INT(10) NOT NULL DEFAULT '0',
  `losses_2p` INT(10) NOT NULL DEFAULT '0',
  `wins_3p` INT(10) NOT NULL DEFAULT '0',
  `losses_3p` INT(10) NOT NULL DEFAULT '0',
  `wins_4p` INT(10) NOT NULL DEFAULT '0',
  `losses_4p` INT(10) NOT NULL DEFAULT '0',
  `forfeits` INT(10) NOT NULL DEFAULT '0',
  `wins_by_forfeit` INT(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `stats_summonwar_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`stats_summonwar_faction_losses`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`stats_summonwar_faction_losses` (
  `user_id` INT(11) UNSIGNED NOT NULL,
  `faction_id` INT(11) UNSIGNED NOT NULL,
  `vs_faction_0` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_1` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_2` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_3` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_4` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_5` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_6` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_7` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_8` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_9` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_10` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_11` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_12` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_13` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_14` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_15` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`, `faction_id`),
  CONSTRAINT `stats_summonwar_faction_losses_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`stats_summonwar_faction_wins`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`stats_summonwar_faction_wins` (
  `user_id` INT(11) UNSIGNED NOT NULL,
  `faction_id` INT(11) UNSIGNED NOT NULL,
  `vs_faction_0` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_1` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_2` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_3` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_4` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_5` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_6` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_7` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_8` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_9` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_10` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_11` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_12` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_13` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_14` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vs_faction_15` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`, `faction_id`),
  CONSTRAINT `stats_summonwar_faction_wins_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`stats_summonwar_vs`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`stats_summonwar_vs` (
  `user_id` INT(11) UNSIGNED NOT NULL,
  `opp_id` INT(11) UNSIGNED NOT NULL,
  `user_wins` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `opp_wins` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`, `opp_id`),
  INDEX `opp_id` (`opp_id` ASC),
  CONSTRAINT `stats_summonwar_vs_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE,
  CONSTRAINT `stats_summonwar_vs_ibfk_2`
    FOREIGN KEY (`opp_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`stats_tantocuore`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`stats_tantocuore` (
  `user_id` INT(10) UNSIGNED NOT NULL,
  `completed_games` INT(10) NOT NULL DEFAULT '0',
  `wins_2p` INT(10) NOT NULL DEFAULT '0',
  `losses_2p` INT(10) NOT NULL DEFAULT '0',
  `wins_3p` INT(10) NOT NULL DEFAULT '0',
  `losses_3p` INT(10) NOT NULL DEFAULT '0',
  `wins_4p` INT(10) NOT NULL DEFAULT '0',
  `losses_4p` INT(10) NOT NULL DEFAULT '0',
  `forfeits` INT(10) NOT NULL DEFAULT '0',
  `wins_by_forfeit` INT(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `stats_tantocuore_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`stats_unsungcard`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`stats_unsungcard` (
  `user_id` INT(10) UNSIGNED NOT NULL,
  `rating` FLOAT NOT NULL,
  `deviation` FLOAT NOT NULL,
  `volatility` FLOAT NOT NULL,
  `completed_games` INT(10) NOT NULL DEFAULT '0',
  `wins_2p` INT(10) NOT NULL DEFAULT '0',
  `losses_2p` INT(10) NOT NULL DEFAULT '0',
  `wins_3p` INT(10) NOT NULL DEFAULT '0',
  `losses_3p` INT(10) NOT NULL DEFAULT '0',
  `wins_4p` INT(10) NOT NULL DEFAULT '0',
  `losses_4p` INT(10) NOT NULL DEFAULT '0',
  `wins_5p` INT(10) NOT NULL DEFAULT '0',
  `losses_5p` INT(10) NOT NULL DEFAULT '0',
  `forfeits` INT(10) NOT NULL DEFAULT '0',
  `wins_by_forfeit` INT(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `stats_unsungcard_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`stats_waterdeep`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`stats_waterdeep` (
  `user_id` INT(10) UNSIGNED NOT NULL,
  `rating` FLOAT NOT NULL,
  `deviation` FLOAT NOT NULL,
  `volatility` FLOAT NOT NULL,
  `completed_games` INT(10) NOT NULL DEFAULT '0',
  `wins_2p` INT(10) NOT NULL DEFAULT '0',
  `losses_2p` INT(10) NOT NULL DEFAULT '0',
  `wins_3p` INT(10) NOT NULL DEFAULT '0',
  `losses_3p` INT(10) NOT NULL DEFAULT '0',
  `wins_4p` INT(10) NOT NULL DEFAULT '0',
  `losses_4p` INT(10) NOT NULL DEFAULT '0',
  `wins_5p` INT(10) NOT NULL DEFAULT '0',
  `losses_5p` INT(10) NOT NULL DEFAULT '0',
  `wins_6p` INT(10) NOT NULL DEFAULT '0',
  `losses_6p` INT(10) NOT NULL DEFAULT '0',
  `forfeits` INT(10) NOT NULL DEFAULT '0',
  `wins_by_forfeit` INT(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `stats_waterdeep_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`string`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`string` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `string` VARCHAR(255) NULL DEFAULT NULL,
  `category` VARCHAR(45) NULL DEFAULT NULL,
  `description` VARCHAR(255) NULL DEFAULT NULL,
  `english` VARCHAR(512) NULL DEFAULT NULL,
  `spanish` VARCHAR(512) NULL DEFAULT NULL,
  `french` VARCHAR(512) NULL DEFAULT NULL,
  `german` VARCHAR(512) NULL DEFAULT NULL,
  `italian` VARCHAR(512) NULL DEFAULT NULL,
  `brazilian_portuguese` VARCHAR(512) NULL DEFAULT NULL,
  `russian` VARCHAR(512) NULL DEFAULT NULL,
  `japanese` VARCHAR(512) NULL DEFAULT NULL,
  `chinese` VARCHAR(512) NULL DEFAULT NULL,
  `replaces` VARCHAR(45) NULL DEFAULT NULL COMMENT '%body% or other string replacements.',
  PRIMARY KEY (`id`))
ENGINE = MyISAM
AUTO_INCREMENT = 159
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`summonwar_event_matchups`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`summonwar_event_matchups` (
  `matchup_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `event_id` INT(10) UNSIGNED NOT NULL,
  `matchup_index` INT(11) UNSIGNED NOT NULL,
  `player_id_1` INT(10) UNSIGNED NOT NULL,
  `player_id_2` INT(10) UNSIGNED NOT NULL,
  `game_id` INT(11) UNSIGNED NOT NULL,
  `winner_id` INT(10) UNSIGNED NULL DEFAULT NULL,
  PRIMARY KEY (`matchup_id`),
  INDEX `summonwar_event_matchups_ibfk_1` (`event_id` ASC))
ENGINE = MyISAM
AUTO_INCREMENT = 5
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`summonwar_events`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`summonwar_events` (
  `event_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `event_type` INT(11) NOT NULL DEFAULT '0',
  `event_state` INT(11) NOT NULL DEFAULT '0',
  `max_players` INT(11) NOT NULL DEFAULT '8',
  `game_params` TINYBLOB NULL DEFAULT NULL,
  `player_timer` INT(11) NOT NULL DEFAULT '0',
  `creation_time` DATETIME NOT NULL,
  `start_time` DATETIME NULL DEFAULT NULL,
  `completion_time` DATETIME NULL DEFAULT NULL,
  `round_number` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `round_matchup_first` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `round_matchup_count` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `total_matchup_count` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`event_id`))
ENGINE = InnoDB
AUTO_INCREMENT = 6
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`summonwar_event_players`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`summonwar_event_players` (
  `player_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `event_id` INT(10) UNSIGNED NOT NULL,
  `user_id` INT(10) UNSIGNED NOT NULL,
  `player_slot` INT(10) UNSIGNED NOT NULL,
  `player_params` TINYBLOB NULL DEFAULT NULL,
  PRIMARY KEY (`player_id`),
  INDEX `summonwar_event_players_ibfk_1` (`user_id` ASC),
  INDEX `summonwar_event_players_ibfk_2` (`event_id` ASC),
  CONSTRAINT `summonwar_event_players_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
  CONSTRAINT `summonwar_event_players_ibfk_2`
    FOREIGN KEY (`event_id`)
    REFERENCES `playdek`.`summonwar_events` (`event_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`third_party_newsletter`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`third_party_newsletter` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `user_email` VARCHAR(60) NULL DEFAULT NULL,
  `third_party_name` VARCHAR(45) NULL DEFAULT 'wotc',
  `timestamp` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE INDEX `user_email` (`user_email` ASC, `third_party_name` ASC))
ENGINE = MyISAM
AUTO_INCREMENT = 15
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`tournament_definition`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`tournament_definition` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `game_id` INT(11) NULL DEFAULT NULL,
  `uuid` VARCHAR(16) NULL DEFAULT '0',
  `name_lookup_string` VARCHAR(45) NULL DEFAULT NULL,
  `announcement_lookup_string` VARCHAR(45) NULL DEFAULT NULL,
  `descrition_lookup_string` VARCHAR(45) NULL DEFAULT NULL,
  `begin_date` DATETIME NULL DEFAULT NULL,
  `end_date` DATETIME NULL DEFAULT NULL,
  `time_per_round` INT(11) NULL DEFAULT '3',
  `time_units_per_round` INT(11) NULL DEFAULT NULL,
  `num_players_per_instance` INT(11) NULL DEFAULT NULL,
  `icon` VARCHAR(45) NULL DEFAULT NULL,
  `image` VARCHAR(45) NULL DEFAULT NULL,
  `play_style` INT(11) NULL DEFAULT '1',
  `games_per_tournament_group` INT(11) NULL DEFAULT '1',
  `wins_to_finish_group` INT(11) NULL DEFAULT '1',
  `group_loses_to_count_as_tournament_loss` INT(11) NULL DEFAULT '0',
  `pd_points_earned_per_win` INT(11) NULL DEFAULT '0',
  `pd_points_earned_per_loss` INT(11) NULL DEFAULT '0',
  `pd_points_earned_per_participate` INT(11) NULL DEFAULT '0',
  `pd_points_earned_per_game_played` INT(11) NULL DEFAULT '0',
  `grouping_rule` INT(11) NULL DEFAULT NULL,
  `games_played_in_parallel` TINYINT(4) NULL DEFAULT '0',
  `are_losers_rematched` TINYINT(4) NULL DEFAULT '0',
  `game_version_minimum` DECIMAL(1,0) NULL DEFAULT NULL,
  `deck_limitations` VARCHAR(45) NULL DEFAULT NULL,
  `does_draft` TINYINT(4) NULL DEFAULT NULL,
  `notes` VARCHAR(100) NULL DEFAULT NULL,
  PRIMARY KEY (`id`))
ENGINE = MyISAM
AUTO_INCREMENT = 2
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`tournament_entrant`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`tournament_entrant` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `user_id` INT(11) NULL DEFAULT '0',
  `tournament_id` INT(11) NULL DEFAULT '0',
  `date_joined` TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP,
  `date_left` DATETIME NULL DEFAULT '0000-00-00 00:00:00',
  `player_withdrew` TINYINT(4) NULL DEFAULT '0',
  `price_paid` INT(11) NULL DEFAULT '1',
  `wins` INT(11) NULL DEFAULT '0',
  `loses` INT(11) NULL DEFAULT '0',
  `forfeits` INT(11) NULL DEFAULT '0',
  `unplayeds` INT(11) NULL DEFAULT '0',
  `highest_tier` INT(11) NULL DEFAULT '0',
  `points_earned_so_far` INT(11) NULL DEFAULT '0',
  `admin_player_kicked` TINYINT(4) NULL DEFAULT '0' COMMENT 'This might be changed into an admin records table',
  PRIMARY KEY (`id`))
ENGINE = MyISAM
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`tournament_grouprule`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`tournament_grouprule` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `group_name` VARCHAR(45) NULL DEFAULT NULL,
  `group_string_lookup` VARCHAR(45) NULL DEFAULT NULL,
  PRIMARY KEY (`id`))
ENGINE = MyISAM
AUTO_INCREMENT = 4
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`tournament_instance`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`tournament_instance` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `tournament_definition_id` INT(11) NULL DEFAULT NULL,
  `uuid` VARCHAR(16) NULL DEFAULT NULL,
  `begin_date_override` DATETIME NULL DEFAULT NULL,
  `end_data_override` DATETIME NULL DEFAULT NULL,
  `has_ended` TINYINT(4) NULL DEFAULT '0',
  `entrance_precondition_id` INT(11) NULL DEFAULT '0',
  `chat_channel_id` VARCHAR(45) NULL DEFAULT NULL COMMENT 'A chat channel specially set up for this tournament instance',
  PRIMARY KEY (`id`))
ENGINE = MyISAM
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`tournament_playstyle`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`tournament_playstyle` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `style` VARCHAR(45) NULL DEFAULT NULL,
  `style_string_lookup` VARCHAR(45) NULL DEFAULT NULL,
  PRIMARY KEY (`id`))
ENGINE = MyISAM
AUTO_INCREMENT = 49
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`user_device`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`user_device` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `user_uuid` VARCHAR(16) NOT NULL,
  `device_uuid` VARCHAR(16) NOT NULL,
  `device_id` TINYBLOB NULL DEFAULT NULL,
  `name` VARCHAR(45) NULL DEFAULT NULL,
  `icon_id` INT(11) NULL DEFAULT '1',
  `platformId` TINYINT(1) NULL DEFAULT '0',
  `user_id` INT(10) UNSIGNED NOT NULL,
  `is_enabled` TINYINT(1) NULL DEFAULT '1',
  `registered_date` TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`))
ENGINE = MyISAM
AUTO_INCREMENT = 225
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`user_device_notification`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`user_device_notification` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `user_device_id` INT(11) NOT NULL COMMENT 'foreign key to the user_device table',
  `game_type` INT(11) NULL DEFAULT NULL,
  `is_enabled` TINYINT(4) NULL DEFAULT '1',
  `time_changed` TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `device_id` TINYBLOB NULL DEFAULT NULL,
  PRIMARY KEY (`id`))
ENGINE = MyISAM
AUTO_INCREMENT = 176
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`user_join_chat_channel`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`user_join_chat_channel` (
  `user_uuid` VARCHAR(16) NOT NULL,
  `channel_uuid` VARCHAR(16) NOT NULL,
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `added_date` TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP,
  `date_last_viewed` DATETIME NULL DEFAULT NULL,
  PRIMARY KEY (`id`))
ENGINE = InnoDB
AUTO_INCREMENT = 509
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`user_join_product`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`user_join_product` (
  `id` INT(4) NOT NULL AUTO_INCREMENT,
  `user_uuid` VARCHAR(16) NULL DEFAULT NULL,
  `product_id` VARCHAR(16) NULL DEFAULT NULL,
  `purchase_date` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `price_paid` DECIMAL(10,0) NULL DEFAULT '0',
  `currency_type` INT(11) NULL DEFAULT NULL,
  `num_purchased` INT(11) NULL DEFAULT '1',
  `admin_provided` VARCHAR(16) NULL DEFAULT '0',
  `admin_notes` VARCHAR(45) NULL DEFAULT NULL,
  `retail_campaign` INT(4) NULL DEFAULT NULL,
  `exchange_rate_id` VARCHAR(16) NULL DEFAULT '0',
  PRIMARY KEY (`id`))
ENGINE = MyISAM
AUTO_INCREMENT = 967
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `playdek`.`user_pending_expired`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`user_pending_expired` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `user_name` VARCHAR(45) NULL DEFAULT NULL,
  `user_id` INT(11) NULL DEFAULT NULL,
  `user_email` VARCHAR(45) NULL DEFAULT NULL,
  `lookup_key` VARCHAR(32) NULL DEFAULT NULL,
  `game_id` INT(11) NULL DEFAULT NULL,
  `time_created` TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP,
  `was_email_sent` INT(11) NULL DEFAULT '0',
  `language_id` INT(11) NULL DEFAULT '1',
  `uuid` VARCHAR(16) NULL DEFAULT NULL,
  `user_gamekit_hash` BIGINT(20) UNSIGNED NULL DEFAULT NULL,
  `user_pw_hash` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
  `user_name_match` VARCHAR(45) NULL DEFAULT NULL,
  `flagged_as_invalid` TINYINT(1) NULL DEFAULT '0',
  `flagged_auto_create` TINYINT(1) NULL DEFAULT '0',
  `time_last_confirmation_email_sent` DATETIME NULL DEFAULT NULL,
  `email_returned_as_undeliverable` TINYINT(1) NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE INDEX `id_UNIQUE` (`id` ASC),
  UNIQUE INDEX `uuid_UNIQUE` (`uuid` ASC))
ENGINE = MyISAM
AUTO_INCREMENT = 229
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`user_profile`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`user_profile` (
  `user_id` INT(11) NOT NULL,
  `address1` VARCHAR(127) NULL DEFAULT NULL,
  `address2` VARCHAR(127) NULL DEFAULT NULL,
  `city` VARCHAR(45) NULL DEFAULT NULL,
  `provence` VARCHAR(45) NULL DEFAULT NULL,
  `mail_code` VARCHAR(12) NULL DEFAULT NULL,
  `country` VARCHAR(30) NULL DEFAULT NULL,
  `marketing_opt_out` TINYINT(1) NULL DEFAULT '0',
  `screen_name` VARCHAR(45) NULL DEFAULT NULL,
  `gender` VARCHAR(1) NULL DEFAULT NULL,
  `mber_avatar` INT(11) NULL DEFAULT NULL,
  `home_phone` VARCHAR(22) NULL DEFAULT NULL,
  `alt_phone` VARCHAR(22) NULL DEFAULT NULL,
  `show_profile_gender` TINYINT(1) NULL DEFAULT '0',
  `admin_level` TINYINT(1) NULL DEFAULT '0',
  `show_win_loss_record` TINYINT(1) NULL DEFAULT '1',
  `time_zone` TINYINT(1) NULL DEFAULT '0',
  `account_create_product_id` INT(11) NULL DEFAULT '0',
  `motto` VARCHAR(80) NULL DEFAULT NULL,
  `display_online_status_to_others` TINYINT(1) NULL DEFAULT '1',
  `block_contact_invitations` TINYINT(1) NULL DEFAULT '0',
  `block_group_invitations` TINYINT(1) NULL DEFAULT '0',
  PRIMARY KEY (`user_id`))
ENGINE = MyISAM
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`user_temp_new_user`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`user_temp_new_user` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `user_name` VARCHAR(45) NULL DEFAULT NULL,
  `user_id` INT(11) NULL DEFAULT NULL,
  `user_email` VARCHAR(45) NULL DEFAULT NULL,
  `lookup_key` VARCHAR(32) NULL DEFAULT NULL,
  `game_id` INT(11) NULL DEFAULT NULL,
  `time_created` TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP,
  `was_email_sent` INT(11) NULL DEFAULT '0',
  `language_id` INT(11) NULL DEFAULT '1',
  `uuid` VARCHAR(16) NULL DEFAULT NULL,
  `user_gamekit_hash` BIGINT(20) UNSIGNED NULL DEFAULT NULL,
  `user_pw_hash` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
  `user_name_match` VARCHAR(45) NULL DEFAULT NULL,
  `flagged_as_invalid` TINYINT(1) NULL DEFAULT '0',
  `flagged_auto_create` TINYINT(1) NULL DEFAULT '0',
  `time_last_confirmation_email_sent` DATETIME NULL DEFAULT NULL,
  `email_returned_as_undeliverable` TINYINT(1) NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE INDEX `id_UNIQUE` (`id` ASC),
  UNIQUE INDEX `uuid_UNIQUE` (`uuid` ASC))
ENGINE = MyISAM
AUTO_INCREMENT = 233
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`users_agricola`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`users_agricola` (
  `user_id` INT(11) UNSIGNED NOT NULL,
  `user_avatar` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `user_rating` INT(11) UNSIGNED NOT NULL DEFAULT '1000',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `users_agricola_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`users_ascension`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`users_ascension` (
  `user_id` INT(11) UNSIGNED NOT NULL,
  `user_avatar` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `user_rating` INT(11) UNSIGNED NOT NULL DEFAULT '1000',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `users_ascension_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`users_fluxx`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`users_fluxx` (
  `user_id` INT(11) UNSIGNED NOT NULL,
  `user_avatar` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `user_rating` INT(11) UNSIGNED NOT NULL DEFAULT '1000',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `users_fluxx_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`users_foodfight`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`users_foodfight` (
  `user_id` INT(11) UNSIGNED NOT NULL,
  `user_avatar` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `user_rating` INT(11) UNSIGNED NOT NULL DEFAULT '1000',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `users_foodfight_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`users_nightfall`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`users_nightfall` (
  `user_id` INT(11) UNSIGNED NOT NULL,
  `user_avatar` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `user_rating` INT(11) UNSIGNED NOT NULL DEFAULT '1000',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `users_nightfall_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`users_pennyarcade`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`users_pennyarcade` (
  `user_id` INT(11) UNSIGNED NOT NULL,
  `user_avatar` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `user_rating` INT(11) UNSIGNED NOT NULL DEFAULT '1000',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `users_pennyarcade_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`users_smashup`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`users_smashup` (
  `user_id` INT(11) UNSIGNED NOT NULL,
  `user_avatar` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `user_rating` INT(11) UNSIGNED NOT NULL DEFAULT '1000',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `users_smashup_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`users_summonwar`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`users_summonwar` (
  `user_id` INT(11) UNSIGNED NOT NULL,
  `user_avatar` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `user_rating` INT(11) UNSIGNED NOT NULL DEFAULT '1000',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `users_summonwar_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`users_tantocuore`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`users_tantocuore` (
  `user_id` INT(11) UNSIGNED NOT NULL,
  `user_avatar` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `user_rating` INT(11) UNSIGNED NOT NULL DEFAULT '1000',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `users_tantocuore_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`users_unsungcard`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`users_unsungcard` (
  `user_id` INT(11) UNSIGNED NOT NULL,
  `user_avatar` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `user_rating` INT(11) UNSIGNED NOT NULL DEFAULT '1000',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `users_unsungcard_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `playdek`.`users_waterdeep`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `playdek`.`users_waterdeep` (
  `user_id` INT(11) UNSIGNED NOT NULL,
  `user_avatar` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `user_rating` INT(11) UNSIGNED NOT NULL DEFAULT '1000',
  PRIMARY KEY (`user_id`),
  CONSTRAINT `users_waterdeep_ibfk_1`
    FOREIGN KEY (`user_id`)
    REFERENCES `playdek`.`users` (`user_id`)
    ON DELETE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
