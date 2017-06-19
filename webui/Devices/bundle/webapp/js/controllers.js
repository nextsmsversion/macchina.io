'use strict';

var devicesControllers = angular.module('devicesControllers', []);

devicesControllers.controller('DevicesCtrl', ['$scope', '$http', '$interval',
  function ($scope, $http, $interval) {
	$scope.states = {};	//
	$scope.states.activeItem = 'PA.instantMessage.1'; //
	
    $scope.devices = [];
    $scope.orderBy = "name";
    
    $scope.sendPaMessage = function() {
    	//alert(document.getElementById('text').value);
    	//alert("Inside $scope.sendPaMessage TODO: pass serviceRegistry");
    	$http.get('/macchina/devices/devices.jss').success(function(data) {
    	      $scope.devices = data;
    	    });
    	//alert("Leave $scope.sendPaMessage");
      }
    
    $scope.setOrderBy = function(col) {
      $scope.orderBy = col;
    }
    $http.get('/macchina/devices/devices.jss').success(function(data) {
      $scope.devices = data;
    });
    /** frequent update of the status
    $interval(function() {
    console.log("frequent update of the status");
      $http.get('/macchina/devices/devices.jss').success(function(data) {
        $scope.devices = data;
      })
    }, 1000);
    **/
  }]);

devicesControllers.controller('SessionCtrl', ['$scope', '$http',
  function($scope, $http) {
    $http.get('/macchina/session.json').success(function(data) {
      $scope.session = data;
      if (!data.authenticated)
      {
        window.location = "/";
      }
    });
  }]);
