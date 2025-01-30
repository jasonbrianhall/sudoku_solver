import os
import json
from PyQt5.QtWidgets import (QDialog, QVBoxLayout, QTabWidget, QTableWidget, 
                           QTableWidgetItem, QHeaderView, QInputDialog, QLineEdit,
                           QLabel, QWidget)
from PyQt5.QtCore import Qt

class LeaderboardTab(QWidget):
    def __init__(self, difficulty, records, parent=None):
        super().__init__(parent)
        layout = QVBoxLayout(self)
        
        # Add difficulty label
        title = QLabel(f"{difficulty.capitalize()} Difficulty")
        title.setStyleSheet("font-size: 14pt; font-weight: bold;")
        title.setAlignment(Qt.AlignCenter)
        layout.addWidget(title)
        
        # Create table
        self.table = QTableWidget()
        self.table.setColumnCount(4)
        self.table.setHorizontalHeaderLabels(['Rank', 'Time', 'Player', 'Date'])
        
        # Set table properties
        header = self.table.horizontalHeader()
        header.setSectionResizeMode(0, QHeaderView.ResizeToContents)
        header.setSectionResizeMode(1, QHeaderView.Stretch)
        header.setSectionResizeMode(2, QHeaderView.Stretch)
        header.setSectionResizeMode(3, QHeaderView.Stretch)
        
        # Populate table
        self.table.setRowCount(len(records))
        for i, record in enumerate(records):
            # Rank
            rank_item = QTableWidgetItem(f"#{i + 1}")
            rank_item.setTextAlignment(Qt.AlignCenter)
            self.table.setItem(i, 0, rank_item)
            
            # Time
            minutes = record['time'] // 60
            seconds = record['time'] % 60
            time_item = QTableWidgetItem(f"{minutes:02d}:{seconds:02d}")
            time_item.setTextAlignment(Qt.AlignCenter)
            self.table.setItem(i, 1, time_item)
            
            # Player
            player_item = QTableWidgetItem(record['initials'])
            player_item.setTextAlignment(Qt.AlignCenter)
            self.table.setItem(i, 2, player_item)
            
            # Date
            date_item = QTableWidgetItem(record['date'])
            date_item.setTextAlignment(Qt.AlignCenter)
            self.table.setItem(i, 3, date_item)
            
            # Highlight gold, silver, bronze
            if i < 3:
                colors = ["#FFD700", "#C0C0C0", "#CD7F32"]
                for col in range(4):
                    self.table.item(i, col).setBackground(colors[i])
        
        layout.addWidget(self.table)

class LeaderboardDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle('Sudoku Leaderboards')
        self.setMinimumSize(600, 400)
        
        layout = QVBoxLayout(self)
        
        # Create tab widget
        self.tabs = QTabWidget()
        
        # Load times and create tabs for each difficulty
        times = load_best_times()
        for difficulty in ['easy', 'medium', 'hard', 'expert', 'extreme']:
            tab = LeaderboardTab(difficulty, times[difficulty])
            self.tabs.addTab(tab, difficulty.capitalize())
        
        layout.addWidget(self.tabs)

def get_times_file_path():
    home_dir = os.path.expanduser('~')
    app_dir = os.path.join(home_dir, '.sudoku')
    if not os.path.exists(app_dir):
        os.makedirs(app_dir)
    return os.path.join(app_dir, 'leaderboards.json')

def load_best_times():
    file_path = get_times_file_path()
    if os.path.exists(file_path):
        with open(file_path, 'r') as f:
            return json.load(f)
    return {
        'easy': [],
        'medium': [],
        'hard': [],
        'expert': [],
        'extreme': []
    }

def get_player_initials(parent):
    while True:
        initials, ok = QInputDialog.getText(
            parent,
            "New High Score!",
            "Enter your initials (2-3 letters):",
            QLineEdit.Normal,
            ""
        )
        
        if not ok:  # User cancelled
            return "AAA"  # Default initials
            
        initials = initials.upper()[:3]  # Convert to uppercase and limit to 3 chars
        if len(initials) >= 2 and initials.isalpha():
            return initials

def save_best_time(parent, difficulty, time):
    from datetime import datetime
    
    times = load_best_times()
    records = times[difficulty]
    
    # Create new record
    new_record = {
        'time': time,
        'initials': get_player_initials(parent),
        'date': datetime.now().strftime('%Y-%m-%d')
    }
    
    # Find position for new record
    position = None
    for i, record in enumerate(records):
        if time < record['time']:
            position = i
            break
    
    if position is not None:
        # Insert at position
        records.insert(position, new_record)
        made_top_10 = True
        rank = position + 1
    elif len(records) < 10:
        # Add to end if less than 10 records
        records.append(new_record)
        made_top_10 = True
        rank = len(records)
    else:
        # Not a top 10 time
        made_top_10 = False
        rank = None
    
    # Keep only top 10
    times[difficulty] = records[:10]
    
    # Save to file
    with open(get_times_file_path(), 'w') as f:
        json.dump(times, f)
    
    return made_top_10, rank, new_record['initials']
