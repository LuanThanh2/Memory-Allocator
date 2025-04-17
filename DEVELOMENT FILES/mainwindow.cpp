#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFont>
#include <QPoint>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    ui->treeWidget->setColumnCount(3);
    ui->treeWidget->setHeaderLabels(QStringList() << "seg" << "base" << "size");
}

MainWindow::~MainWindow() {
    delete scene; // Giải phóng scene để tránh rò rỉ bộ nhớ
    delete ui;
}

// Cập nhật giao diện
void MainWindow::updateDisplay() {
    scene->clear();

    // Lấy dữ liệu từ memory_manager.c
    block* free_blocks;
    int free_count = get_free_blocks(&free_blocks);
    block* alloc_blocks;
    int alloc_count = get_alloc_blocks(&alloc_blocks);
    proj* processes;
    int proc_count = get_processes(&processes);

    // Vẽ các lỗ trống
    for (int i = 0; i < free_count; i++) {
        QString strInfo = QString(free_blocks[i].name) + " " +
                          QString::number(free_blocks[i].start) + " " +
                          QString::number(free_blocks[i].end) + " " +
                          QString::number(free_blocks[i].size);
        scene->addText(strInfo, QFont("Segoe UI", 10, QFont::Bold))
            ->setPos(QPoint(100, free_blocks[i].start));
        squer = new MySquer(free_blocks[i], true); // Sửa MySquare thành MySquer
        scene->addItem(squer);
    }

    // Vẽ các khối đã cấp phát
    for (int i = 0; i < alloc_count; i++) {
        if (strcmp(alloc_blocks[i].name, "alloc for system") == 0) {
            QString strInfo = QString(alloc_blocks[i].name) + " " +
                              QString::number(alloc_blocks[i].start) + " " +
                              QString::number(alloc_blocks[i].end) + " " +
                              QString::number(alloc_blocks[i].size);
            scene->addText(strInfo, QFont("Segoe UI", 10, QFont::Bold))
                ->setPos(QPoint(100, alloc_blocks[i].start));
            squer = new MySquer(alloc_blocks[i], false); // Sửa MySquare thành MySquer
            scene->addItem(squer);
        }
    }

    // Vẽ các tiến trình đã cấp phát
    for (int i = 0; i < proc_count; i++) {
        if (processes[i].allocated) {
            for (int j = 0; j < processes[i].segs_count; j++) {
                QString strInfo = QString(processes[i].name) +
                                  " has seg " + QString(processes[i].segs[j].name) +
                                  " from " + QString::number(processes[i].segs[j].start) +
                                  " to " + QString::number(processes[i].segs[j].end) +
                                  " " + QString::number(processes[i].segs[j].size);
                scene->addText(strInfo, QFont("Segoe UI", 10, QFont::Bold))
                    ->setPos(QPoint(100, processes[i].segs[j].start));
                squer = new MySquer(processes[i].segs[j], false); // Sửa MySquare thành MySquer
                scene->addItem(squer);
            }
        }
    }
}

// Khởi tạo bộ nhớ
void MainWindow::on_lineEdit_memSize_editingFinished() {
    int size = ui->lineEdit_memSize->text().toInt();
    init_memory(size);
    ui->treeWidget->clear();
    updateDisplay();
}

// Thêm lỗ trống
void MainWindow::on_pushButton_Addhole_clicked() {
    int start = ui->lineEdit_holestart->text().toInt();
    int size = ui->lineEdit_holeSize->text().toInt();
    add_hole(start, size);
    updateDisplay();
}

// Giải phóng block
void MainWindow::on_pushButton_4_clicked() {
    int pos = ui->lineEdit_dealloc->text().toInt();
    bool found = false;
    block* alloc_blocks;
    int alloc_count = get_alloc_blocks(&alloc_blocks);
    for (int i = 0; i < alloc_count; i++) {
        if (pos == alloc_blocks[i].start) {
            if (strcmp(alloc_blocks[i].name, "alloc for system") == 0) {
                deallocate_block(pos);
            } else {
                QMessageBox::information(this, "info", "This part for a process can't be free");
            }
            found = true;
            break;
        }
    }
    if (!found) {
        QMessageBox::information(this, "info", "Block not found");
    }
    updateDisplay();
}

// Thêm tiến trình
void MainWindow::on_pushButton_addPocess_clicked() {
    if (ui->lineEdit_processName->text().isEmpty() || ui->lineEdit_Numseg->text().isEmpty()) {
        QMessageBox::warning(this, "", "Make sure you entered process name and segments number");
        return;
    }

    QString process = ui->lineEdit_processName->text();
    int num_seg = ui->lineEdit_Numseg->text().toInt();
    add_process(process.toStdString().c_str(), num_seg);
    
    proj* processes;
    int proc_count = get_processes(&processes);
    bool exists = false;
    for (int i = 0; i < proc_count; i++) {
        if (strcmp(processes[i].name, process.toStdString().c_str()) == 0) {
            if (processes[i].num_seg == num_seg) {
                ui->comboBox_process->addItem(process);
            } else {
                QMessageBox::warning(this, "warning", "This process already exists");
            }
            exists = true;
            break;
        }
    }
    if (!exists) {
        QMessageBox::warning(this, "warning", "Failed to add process");
    }
}

// Thêm segment vào tiến trình
void MainWindow::on_pushButton_5_clicked() {
    QString process = ui->comboBox_process->currentText();
    add_segment_to_process(process.toStdString().c_str(),
                          ui->lineEdit_Segname->text().toStdString().c_str(),
                          ui->lineEdit_Segsize->text().toInt());
    
    proj* processes;
    int proc_count = get_processes(&processes);
    for (int i = 0; i < proc_count; i++) {
        if (strcmp(processes[i].name, process.toStdString().c_str()) == 0) {
            if (processes[i].segs_count >= processes[i].num_seg) {
                QMessageBox::information(this, "", "This process has the required number of segments");
            }
            break;
        }
    }
}

// Cấp phát tiến trình
void MainWindow::on_pushButton_3_clicked() {
    QString process = ui->comboBox_process->currentText();
    QString op = ui->comboBox_operation->currentText();
    allocate_process(process.toStdString().c_str(), op.toStdString().c_str());

    proj* processes;
    int proc_count = get_processes(&processes);
    for (int i = 0; i < proc_count; i++) {
        if (strcmp(processes[i].name, process.toStdString().c_str()) == 0) {
            if (processes[i].segs_count < processes[i].num_seg) {
                QMessageBox::information(this, "error", "This process has only " +
                                         QString::number(processes[i].segs_count) +
                                         " and it should have " +
                                         QString::number(processes[i].num_seg));
                return;
            }
            if (processes[i].allocated) {
                ui->comboBox_allocated_porcess->addItem(process);
                ui->comboBox_process->removeItem(ui->comboBox_process->currentIndex());
                QTreeWidgetItem *itm = new QTreeWidgetItem(ui->treeWidget);
                itm->setText(0, process);
                ui->treeWidget->addTopLevelItem(itm);
                for (int j = 0; j < processes[i].segs_count; j++) {
                    QTreeWidgetItem *itm2 = new QTreeWidgetItem();
                    itm2->setText(0, QString::fromStdString(processes[i].segs[j].name));
                    itm2->setText(1, QString::number(processes[i].segs[j].start));
                    itm2->setText(2, QString::number(processes[i].segs[j].size));
                    itm->addChild(itm2);
                }
            } else {
                QMessageBox::information(this, "notification", "This process can't be allocated, deallocate and try again");
            }
            break;
        }
    }
    updateDisplay();
}

// Giải phóng tiến trình
void MainWindow::on_pushButton_dealloc_process_clicked() {
    QString process = ui->comboBox_allocated_porcess->currentText();
    deallocate_process(process.toStdString().c_str());
    ui->comboBox_allocated_porcess->removeItem(ui->comboBox_allocated_porcess->currentIndex());
    updateDisplay();
}

// Giải phóng tiến trình từ tree widget
void MainWindow::on_treeWidget_doubleClicked(const QModelIndex &index) {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "", "Would you want to deallocate this process from the memory!!",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QString process = ui->treeWidget->currentItem()->text(0);
        deallocate_process(process.toStdString().c_str());
        delete ui->treeWidget->currentItem();
        for (int i = 0; i < ui->comboBox_allocated_porcess->count(); i++) {
            if (ui->comboBox_allocated_porcess->itemText(i) == process) {
                ui->comboBox_allocated_porcess->removeItem(i);
                break;
            }
        }
        updateDisplay();
    }
}

// Các slot không được sử dụng (có thể xóa nếu không cần)
void MainWindow::on_pushButton_clicked() {
    // Chưa được triển khai
}

void MainWindow::on_pushButton_add_clicked() {
    // Chưa được triển khai
}

void MainWindow::on_comboBox_process_currentTextChanged(const QString &arg1) {
    // Chưa được triển khai
}