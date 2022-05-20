/**
 * @file main.cpp
 * @author Ninad Harishchandrakar (ninadh@umd.edu), Prasanna Raghavan (pthiruku@umd.edu), Sai Sandeep Adapa (sadapa@umd.edu)
 * @brief Node to run final_project
 * @version 0.1
 * @date 2021-12-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <ros/ros.h>
#include <actionlib/client/simple_action_client.h>
#include <fiducial_msgs/FiducialTransformArray.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <std_msgs/String.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/Twist.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_ros/transform_broadcaster.h>
#include <tf2_ros/transform_listener.h>
#include <nav_msgs/Odometry.h>
#include <array>
#include <sstream>


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient; //client to send goal

// Variables for storing information from callback function for the subscriber to the topic /fiducial_transforms
bool aruco_marker_found{false};
int aruco_id{};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * @brief Function to change the target location fed to the explorer at each iteration
 * 
 * @param explorer_goals 
 * @param goalCounter 
 * @return move_base_msgs::MoveBaseGoal 
 */
move_base_msgs::MoveBaseGoal changeGoal(std::vector<XmlRpc::XmlRpcValue> explorer_goals, int goalCounter) 
{
    move_base_msgs::MoveBaseGoal explorer_goal;

    explorer_goal.target_pose.header.frame_id = "map";
    explorer_goal.target_pose.header.stamp = ros::Time::now();
    explorer_goal.target_pose.pose.position.x = explorer_goals[goalCounter][0];
    explorer_goal.target_pose.pose.position.y = explorer_goals[goalCounter][1];
    explorer_goal.target_pose.pose.orientation.w = 1.0;

    return explorer_goal;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * @brief Callback function for the subscriber to the topic /fiducial_transforms
 * 
 * @param msg 
 */
void fiducial_callback(const fiducial_msgs::FiducialTransformArray::ConstPtr& msg)
{   
    if (!msg->transforms.empty() && (msg->transforms[0].transform.translation.x < 3)) {//check marker is detected
        
        aruco_marker_found = true; //set flag when Aruco Marker is found

        //broadcaster object
        static tf2_ros::TransformBroadcaster br;

        //broadcast the new frame to /tf Topic
        geometry_msgs::TransformStamped transformStamped;
        //store the transformation of the marker with respect to the camera frame
        transformStamped.header.stamp = ros::Time::now();
        transformStamped.header.frame_id = "explorer_tf/camera_rgb_optical_frame";
        transformStamped.child_frame_id = "marker_frame"; //name of the frame
        transformStamped.transform.translation.x = msg->transforms[0].transform.translation.x;
        transformStamped.transform.translation.y = msg->transforms[0].transform.translation.y;
        transformStamped.transform.translation.z = msg->transforms[0].transform.translation.z;
        transformStamped.transform.rotation.x = msg->transforms[0].transform.rotation.x;
        transformStamped.transform.rotation.y = msg->transforms[0].transform.rotation.y;
        transformStamped.transform.rotation.z = msg->transforms[0].transform.rotation.z;
        transformStamped.transform.rotation.w = msg->transforms[0].transform.rotation.w;
        br.sendTransform(transformStamped); //broadcast the transform on /tf Topic

        aruco_id = msg->transforms[0].fiducial_id; //store the ID of the Aruco Marker

    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int main(int argc, char **argv)
{

  ros::init(argc, argv, "final_project_node");
  ros::NodeHandle nh;

  //Flags for sequential code
  bool explorer_goal_sent{false};
  bool follower_goal_sent{false};
  bool explorer_goal_reached{false};
  bool explorer_work_done{false};

  //Goal counters for the explorer and follower
  int explorer_goalCounter{0};
  int follower_goalCounter{0};

  //Objects of client
  MoveBaseClient explorer_client("/explorer/move_base", true);
  MoveBaseClient follower_client("/follower/move_base", true);

  std::vector<XmlRpc::XmlRpcValue> explorer_goals; //explorer goal positions
  std::array<std::pair<double, double>, 4> aruco_location{}; //locations of Aruco markers

  geometry_msgs::TransformStamped transformStamped;

  //Objects for listener
  tf2_ros::Buffer tfBuffer;
  tf2_ros::TransformListener tfListener(tfBuffer);
  
  //Explorer home position
  move_base_msgs::MoveBaseGoal explorer_home;
  explorer_home.target_pose.header.frame_id = "map";
  explorer_home.target_pose.header.stamp = ros::Time::now();
  explorer_home.target_pose.pose.position.x = -4.0;
  explorer_home.target_pose.pose.position.y = 2.5;
  explorer_home.target_pose.pose.orientation.w = 1.0;

  //Follower goal
  move_base_msgs::MoveBaseGoal follower_goal;
  follower_goal.target_pose.header.frame_id = "map";
  follower_goal.target_pose.header.stamp = ros::Time::now();
  follower_goal.target_pose.pose.orientation.w = 1.0;


  //Define publisher and subscriber
  ros::Publisher explorer_rotate_pub = nh.advertise<geometry_msgs::Twist>("explorer/cmd_vel", 1);
  ros::Subscriber sub_odom = nh.subscribe("/fiducial_transforms", 1000, &fiducial_callback);


  //Data to be published onto the topic explorer/cmd_vel
  geometry_msgs::Twist explorer_rotate;
  geometry_msgs::Twist explorer_stop;
  explorer_rotate.angular.z = 0.1;
  explorer_stop.angular.z = 0;



  //Get explorer target locations from the parameter server and put them into a vector
  XmlRpc::XmlRpcValue explorer_target_1{};
  XmlRpc::XmlRpcValue explorer_target_2{};
  XmlRpc::XmlRpcValue explorer_target_3{};
  XmlRpc::XmlRpcValue explorer_target_4{};

  ros::param::get("aruco_lookup_locations/target_1", explorer_target_1);
  ros::param::get("aruco_lookup_locations/target_2", explorer_target_2);
  ros::param::get("aruco_lookup_locations/target_3", explorer_target_3);
  ros::param::get("aruco_lookup_locations/target_4", explorer_target_4);

  explorer_goals = {explorer_target_1, explorer_target_2, explorer_target_3, explorer_target_4};




  while (!explorer_client.waitForServer(ros::Duration(5.0))) 
  {
  ROS_INFO("Waiting for the move_base action server to come up for explorer");
  }

  while (!follower_client.waitForServer(ros::Duration(5.0))) {
  ROS_INFO("Waiting for the move_base action server to come up for follower");
  }



  ros::Rate loop_rate(10);


  while (ros::ok())
  {
    if(explorer_goalCounter < 4) {
      
      //Send goal to explorer
      if (!explorer_goal_sent)     
      {
          ROS_INFO("Sending goal for explorer");
          explorer_client.sendGoal(changeGoal(explorer_goals, explorer_goalCounter));
          explorer_goal_sent = true;
      }

      //After reaching goal, set flags
      if (explorer_client.getState() == actionlib::SimpleClientGoalState::SUCCEEDED && !explorer_goal_reached) 
      {
          ROS_INFO("GOAL REACHED");
          explorer_goal_reached = true;
          aruco_marker_found = false;
      }

      if(explorer_goal_reached)
      {
          explorer_rotate_pub.publish(explorer_rotate); //rotate explorer

          if(aruco_marker_found) {

              explorer_rotate_pub.publish(explorer_stop); //stop rotating explorer after Aruco Marker is found

              //listen to /tf Topic and get the broadcast frame in the /map frame
              try {
                  transformStamped = tfBuffer.lookupTransform("map", "marker_frame", ros::Time(0), ros::Duration(4.0));
                  ROS_INFO_STREAM("marker in /map frame: [" << transformStamped.transform.translation.x << "," << transformStamped.transform.translation.y << "," << transformStamped.transform.translation.z << "]" );
              
                  aruco_location[aruco_id] = std::make_pair(transformStamped.transform.translation.x, transformStamped.transform.translation.y); //Add location of Aruco Markers to the location arrayy based on their ID           
              }
              catch (tf2::TransformException& ex) {
                  ROS_WARN("%s", ex.what());
                  ros::Duration(1.0).sleep();
              }
            
            explorer_goalCounter++; //Increment explorer goal counter

            //Set flags
            explorer_goal_reached = false;
            explorer_goal_sent = false;
          }
      }
    }

    //Send explorer to home position after visiting the 4 target locations
    else {

      if (!explorer_goal_sent && !explorer_work_done)     
      {
          ROS_INFO("Sending goal for explorer");
          explorer_client.sendGoal(explorer_home);//this should be sent only once
          explorer_goal_sent = true;
      }

      if (explorer_client.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) 
      {
          ROS_INFO("EXPLORER WORK DONE");
          explorer_work_done = true;
      }

    }


    //Once the work of the explorer is done, send the follower to the locations of the Aruco markers
    if(explorer_work_done)
    {
      if(follower_goalCounter < 4)
      {
        //Set follower target with a tolerance of 0.4m
        follower_goal.target_pose.pose.position.x = (aruco_location[follower_goalCounter].first - 0.4);
        follower_goal.target_pose.pose.position.y = (aruco_location[follower_goalCounter].second -0.4);

        if (!follower_goal_sent)     
        {
          ROS_INFO("Sending goal for follower");
          follower_client.sendGoal(follower_goal);
          follower_goal_sent = true;
        }

        if (follower_client.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) 
        {
          ROS_INFO("Follower Goal Reached");
          follower_goalCounter++;
          follower_goal_sent = false;
        }
      }

      //Once all 4 locations have been visited, send the follower to its home location and shut the node down once reached
      else {
        follower_goal.target_pose.pose.position.x = -4.0;
        follower_goal.target_pose.pose.position.y = 3.5;

        if (!follower_goal_sent)     
        {
          ROS_INFO("Sending HOME location for follower");
          follower_client.sendGoal(follower_goal);
          follower_goal_sent = true;
        }

        if (follower_client.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) 
        {
          ROS_INFO("Follower HOME Reached, shutting Node down");
          ros::Duration(5).sleep();
          ros::shutdown();
        }
      }
    }



    loop_rate.sleep();
    ros::spinOnce();
  }

  return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
