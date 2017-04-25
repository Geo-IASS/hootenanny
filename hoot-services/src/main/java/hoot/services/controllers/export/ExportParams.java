/*
 * This file is part of Hootenanny.
 *
 * Hootenanny is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * --------------------------------------------------------------------
 *
 * The following copyright notices are generated automatically. If you
 * have a new notice to add, please use the format:
 * " * @copyright Copyright ..."
 * This will properly maintain the copyright information. DigitalGlobe
 * copyrights will be updated automatically.
 *
 * @copyright Copyright (C) 2015, 2016, 2017 DigitalGlobe (http://www.digitalglobe.com/)
 */
package hoot.services.controllers.export;


import com.fasterxml.jackson.annotation.JsonProperty;


public class ExportParams {

    @JsonProperty("outputtype")
    private String outputType;

    @JsonProperty("outputname")
    private String outputName;

    @JsonProperty("textstatus")
    private Boolean textStatus;

    @JsonProperty("inputtype")
    private String inputType;

    @JsonProperty("TASK_BBOX")
    private String bounds;

    @JsonProperty("USER_ID")
    private String userId;

    @JsonProperty("USER_EMAIL")
    private String userEmail;

    @JsonProperty("input")
    private String input;

    @JsonProperty("translation")
    private String translation;

    @JsonProperty("append")
    private Boolean append;

    @JsonProperty("input1")
    private String input1;

    @JsonProperty("input2")
    private String input2;

    public String getOutputType() {
        return outputType;
    }

    public void setOutputType(String outputType) {
        this.outputType = outputType;
    }

    public String getOutputName() {
        return outputName;
    }

    public void setOutputName(String outputName) {
        this.outputName = outputName;
    }

    public String getInput() {
        return input;
    }

    public void setInput(String input) {
        this.input = input;
    }

    public String getTranslation() {
        return translation;
    }

    public void setTranslation(String translation) {
        this.translation = translation;
    }

    public Boolean getTextStatus() {
        return textStatus;
    }

    public void setTextStatus(Boolean textStatus) {
        this.textStatus = textStatus;
    }

    public Boolean getAppend() {
        return append;
    }

    public void setAppend(Boolean append) {
        this.append = append;
    }

    public String getInputType() {
        return inputType;
    }

    public void setInputType(String inputType) {
        this.inputType = inputType;
    }

    public String getBounds() {
        return bounds;
    }

    public void setBounds(String bounds) {
        this.bounds = bounds;
    }

    public String getUserId() {
        return userId;
    }

    public void setUserId(String userId) {
        this.userId = userId;
    }

    public String getUserEmail() {
        return userEmail;
    }

    public void setUserEmail(String userEmail) {
        this.userEmail = userEmail;
    }

    public String getInput1() {
        return input1;
    }

    public void setInput1(String input1) {
        this.input1 = input1;
    }

    public String getInput2() {
        return input2;
    }

    public void setInput2(String input2) {
        this.input2 = input2;
    }

    @Override
    public String toString() {
        return "ExportParams{" +
                "outputType='" + outputType + '\'' +
                ", outputName='" + outputName + '\'' +
                ", textStatus=" + textStatus +
                ", inputType='" + inputType + '\'' +
                ", bounds='" + bounds + '\'' +
                ", userId='" + userId + '\'' +
                ", userEmail='" + userEmail + '\'' +
                ", input='" + input + '\'' +
                ", translation='" + translation + '\'' +
                ", append=" + append +
                ", input1='" + input1 + '\'' +
                ", input2='" + input2 + '\'' +
                '}';
    }
}
