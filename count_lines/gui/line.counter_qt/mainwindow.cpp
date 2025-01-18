#include "mainwindow.h"
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QTextEdit>
#include <QClipboard>
#include <QApplication>
#include <QDirIterator>

#include <fstream>

Worker::Worker(QObject *parent) : QObject(parent) {}

void Worker::setParameters(const QString &directory, const QStringList &extensions)
{
    m_directory = directory;
    m_extensions = extensions;
}

void Worker::process()
{
    QStringList files = gatherFiles(m_directory, m_extensions);
    unsigned long totalLines = 0;
    unsigned long totalBlank = 0;
    QString result;

    if (files.isEmpty()) {
        result = "<p style='color:red'>No matching files found.</p>";
    } else {
        result.append("<p>Found <b>" + QString::number(files.size()) + "</b> matching files:</p><ul>");
        for (const QString &file : files) {
            unsigned long blankLines = 0;
            unsigned long lineCount = countFile(file, blankLines);
            totalLines += lineCount;
            totalBlank += blankLines;

            result.append("<li><span style='color:green'>" + file + "</span> : "
                          "<b>" + QString::number(lineCount) + "</b> lines, "
                          "<b>" + QString::number(blankLines) + "</b> blank</li>");
        }
        result.append("</ul>");
        result.append("<p>Total lines: <span style='color:blue'><b>" +
                      QString::number(totalLines) + "</b></span></p>");
        result.append("<p>Total blank lines: <span style='color:blue'><b>" +
                      QString::number(totalBlank) + "</b></span></p>");
        result.append("<p style='color:purple'>Combined total: <b>" +
                      QString::number(totalLines + totalBlank) + "</b></p>");
    }

    emit resultReady(result);
    emit finished();
}

QStringList Worker::gatherFiles(const QString &directory, const QStringList &extensions)
{
    QStringList matchedFiles;
    QDirIterator it(directory, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        QString fileExt = QFileInfo(filePath).suffix().toLower();
        for (const QString &ext : extensions) {
            QString cleanExt = ext.trimmed();
            if (cleanExt.startsWith('.')) {
                cleanExt.remove(0, 1);
            }

            // Debugging print
            qDebug() << "Checking file:" << filePath << "extension:" << fileExt << "against:" << cleanExt;

            if (fileExt == cleanExt) {
                matchedFiles.append(filePath);
                emit progressUpdate("Found file: " + filePath);
                break;
            }
        }
    }
    return matchedFiles;
}

unsigned long Worker::countFile(const QString &filePath, unsigned long &blankLines)
{
    unsigned long counter = 0;
    blankLines = 0;

    std::ifstream inFile(filePath.toStdString());
    if (!inFile.is_open()) {
        emit progressUpdate("Could not open file: " + filePath);
        return 0;
    }

    std::string line;
    while (std::getline(inFile, line)) {
        if (lineEmpty(QString::fromStdString(line))) {
            ++blankLines;
        } else {
            ++counter;
        }
    }
    return counter;
}

bool Worker::lineEmpty(const QString &line) const
{
    return line.trimmed().isEmpty();
}

 

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(nullptr)
{
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    QHBoxLayout *dirLayout = new QHBoxLayout;
    QLabel *dirLabel = new QLabel("Directory:", this);
    QLineEdit *dirEdit = new QLineEdit(this);
    dirEdit->setObjectName("dirEdit");
    QPushButton *browseButton = new QPushButton("Browse...", this);
    connect(browseButton, &QPushButton::clicked, this, &MainWindow::onBrowseDirectory);

    dirLayout->addWidget(dirLabel);
    dirLayout->addWidget(dirEdit);
    dirLayout->addWidget(browseButton);

    QHBoxLayout *extLayout = new QHBoxLayout;
    QLabel *extLabel = new QLabel("Extensions (e.g. .cpp; .h; .hpp):", this);
    QLineEdit *extEdit = new QLineEdit(this);
    extEdit->setObjectName("extEdit");
    extEdit->setPlaceholderText(".cpp; .h; .hpp; ...");

    extLayout->addWidget(extLabel);
    extLayout->addWidget(extEdit);


    QPushButton *countButton = new QPushButton("Count Lines", this);
    connect(countButton, &QPushButton::clicked, this, &MainWindow::onCountLines);

    QTextEdit *resultsEdit = new QTextEdit(this);
    resultsEdit->setObjectName("resultsEdit");
    resultsEdit->setReadOnly(true);

    QPushButton *copyButton = new QPushButton("Copy Results", this);
    connect(copyButton, &QPushButton::clicked, this, &MainWindow::onCopyResults);

    mainLayout->addLayout(dirLayout);
    mainLayout->addLayout(extLayout);
    mainLayout->addWidget(countButton);
    mainLayout->addWidget(resultsEdit);
    mainLayout->addWidget(copyButton);

    setCentralWidget(central);
    setWindowTitle("Line Counter");
    setGeometry(150, 150, 800, 600);

    workerThread = new QThread(this);
    worker = new Worker();
    worker->moveToThread(workerThread);

    connect(workerThread, &QThread::started, worker, &Worker::process);
    connect(worker, &Worker::progressUpdate, this, &MainWindow::handleProgressUpdate);
    connect(worker, &Worker::resultReady, this, &MainWindow::handleResultReady);
    connect(worker, &Worker::finished, this, &MainWindow::handleWorkerFinished);
}

MainWindow::~MainWindow()
{
    workerThread->quit();
    workerThread->wait();
    delete worker;
}

void MainWindow::onBrowseDirectory()
{
    QLineEdit *dirEdit = findChild<QLineEdit*>("dirEdit");
    if (!dirEdit) return;

    QString dirPath = QFileDialog::getExistingDirectory(this, "Select Directory");
    if (!dirPath.isEmpty()) {
        dirEdit->setText(dirPath);
    }
}

void MainWindow::onCountLines()
{
    QLineEdit *dirEdit = findChild<QLineEdit*>("dirEdit");
    QLineEdit *extEdit = findChild<QLineEdit*>("extEdit");
    if (!dirEdit || !extEdit) return;

    QString directory = dirEdit->text();
    QString extensions = extEdit->text();
    QStringList extList = extensions.split(';', Qt::SkipEmptyParts);

    worker->setParameters(directory, extList);

    workerThread->start();
}

void MainWindow::onCopyResults()
{
    QTextEdit *resultsEdit = findChild<QTextEdit*>("resultsEdit");
    if (!resultsEdit) return;

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(resultsEdit->toPlainText());
}

void MainWindow::handleProgressUpdate(const QString &message)
{
    appendResult("<p>" + message + "</p>");
}

void MainWindow::handleResultReady(const QString &result)
{
    QTextEdit *resultsEdit = findChild<QTextEdit*>("resultsEdit");
    if (resultsEdit) {
        resultsEdit->setHtml(result);
    }
}

void MainWindow::handleWorkerFinished()
{
    workerThread->quit();
}

void MainWindow::appendResult(const QString &text)
{
    QTextEdit *resultsEdit = findChild<QTextEdit*>("resultsEdit");
    if (resultsEdit) {
        resultsEdit->append(text);
    }
}
