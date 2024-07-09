/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the FOO module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

var ComponentSelectionPage = null;

var Dir = new function () {
    this.toNativeSparator = function (path) {
        if (systemInfo.productType === "windows")
            return path.replace(/\//g, '\\');
        return path;
    }
};

function Component() {
    if (installer.isInstaller()) {
        component.loaded.connect(this, Component.prototype.installerLoaded);
        ComponentSelectionPage = gui.pageById(QInstaller.ComponentSelection);

        installer.setDefaultPageVisible(QInstaller.TargetDirectory, false);
        installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
        installer.setDefaultPageVisible(QInstaller.LicenseCheck, false);
        installer.setDefaultPageVisible(QInstaller.ReadyForInstallation, false);
    }
}

Component.prototype.installerLoaded = function () {
    if (installer.addWizardPage(component, "TargetWidget", QInstaller.TargetDirectory)) {
        var widget = gui.pageWidgetByObjectName("DynamicTargetWidget");
        if (widget != null) {
            widget.targetChooser.clicked.connect(this, Component.prototype.chooseTarget);
            widget.targetDirectory.textChanged.connect(this, Component.prototype.targetChanged);

            widget.windowTitle = "Installation Folder";
            widget.targetDirectory.text = Dir.toNativeSparator(installer.value("TargetDir"));
        }
    }

    if (installer.addWizardPage(component, "InstallationWidget", QInstaller.ComponentSelection)) {
        var widget = gui.pageWidgetByObjectName("DynamicInstallationWidget");
        if (widget != null) {
            widget.studentInstall.toggled.connect(this, Component.prototype.studentInstallToggled);
            widget.teacherInstall.toggled.connect(this, Component.prototype.teacherInstallToggled);
            widget.studentInstall.checked = true;
            widget.passwordGroup.setVisible(false);
            widget.passwordGroup.passwordAccept.clicked.connect(this, Component.prototype.onPasswordAccepted);
            widget.passwordGroup.password.returnPressed.connect(this, Component.prototype.onPasswordAccepted);

            widget.windowTitle = "Select Installation Type";
        }

        if (installer.addWizardPage(component, "ReadyToInstallWidget", QInstaller.ReadyForInstallation)) {
            var widget = gui.pageWidgetByObjectName("DynamicReadyToInstallWidget");
            if (widget != null) {
                widget.showDetails.checked = false;
                widget.windowTitle = "Ready to Install";
            }
            var page = gui.pageByObjectName("DynamicReadyToInstallWidget");
            if (page != null) {
                page.entered.connect(this, Component.prototype.readyToInstallWidgetEntered);
            }
        }
    }
    component.languageChanged();
}

Component.prototype.targetChanged = function (text) {
    var widget = gui.pageWidgetByObjectName("DynamicTargetWidget");
    if (widget != null) {
        if (text != "") {
            if (!installer.fileExists(text + "/components.xml")) {
                widget.complete = true;
                installer.setValue("TargetDir", text);
                return;
            }
        }
        widget.complete = false;
    }
}

Component.prototype.chooseTarget = function () {
    var widget = gui.pageWidgetByObjectName("DynamicTargetWidget");
    if (widget != null) {
        var newTarget = QFileDialog.getExistingDirectory("Choose your target directory.", widget
            .targetDirectory.text);
        if (newTarget != "")
            widget.targetDirectory.text = Dir.toNativeSparator(newTarget);
    }
}

Component.prototype.studentInstallToggled = function (checked) {
    if (checked) {
        if (ComponentSelectionPage != null){
            ComponentSelectionPage.selectComponent("main.Learner");
            ComponentSelectionPage.deselectComponent("main.LearnerAndEditor");
        }
        var widget = gui.pageWidgetByObjectName("DynamicInstallationWidget");
        if(widget != null){
            widget.passwordGroup.setVisible(false);
            widget.complete = true;
        }
    }
}

Component.prototype.teacherInstallToggled = function (checked) {
    if (checked) {
        if (ComponentSelectionPage != null){
            ComponentSelectionPage.selectComponent("main.LearnerAndEditor");
            ComponentSelectionPage.deselectComponent("main.Learner");
        }
        var widget = gui.pageWidgetByObjectName("DynamicInstallationWidget");
        if(widget != null){
            widget.passwordGroup.setVisible(true);
            widget.complete = false;
        }
    }
}

Component.prototype.onPasswordAccepted = function (){
    var widget = gui.pageWidgetByObjectName("DynamicInstallationWidget");
    if(widget != null){
        //This is generally a most terrible way to do password auth, but it's only there so people aren't confused by the edit options.
        //Having access to the Editor doesn't enable anyone to upload anything but only to easily edit 
        //the sets they lear locally.

//the old password was phoenixSoftwareWin
        if(widget.passwordGroup.password.text === "geoLearnPasswdWin2024") widget.complete = true;
        else{
            widget.passwordGroup.password.text = "";
            widget.complete = false;
        }
    }
}

Component.prototype.readyToInstallWidgetEntered = function () {
    var widget = gui.pageWidgetByObjectName("DynamicReadyToInstallWidget");
    if (widget != null) {
        var html = "<b>Components to install:</b><ul>";
        var components = installer.components();
        for (i = 0; i < components.length; ++i) {
            if (components[i].installationRequested())
                html = html + "<li>" + components[i].displayName + "</li>"
        }
        html = html + "</ul>";
        widget.showDetailsBrowser.html = html;
    }
}

Component.prototype.createOperations = function()
{
    // call default implementation to actually install the GeographyLearner
    component.createOperations();

    if (systemInfo.productType === "windows") {
        component.addOperation("CreateShortcut", "@TargetDir@/GeographyLearner.exe", "@StartMenuDir@/GeographyLearner.lnk",
            "workingDirectory=@TargetDir@", "iconPath=@TargetDir@/Globe.ico",
            "iconId=0", "description=Open the GeographyLearner");
    }

}
