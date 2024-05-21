function Component() {
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
