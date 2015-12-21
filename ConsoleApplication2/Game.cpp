#include "Game.h"

Game::Game() : init_point(50, 346), dir_resources("./resources/"), dir_scenes("./scenes/") {}

Game::~Game() {}

void Game::Start(const bool &visualize, const int &start_index, const int &animation_speed)
{
    if (start_index < 0 || start_index > 99)
    {
        printf("Error: Invalid starting index! Please enter between 0 and 99.\n");
        return;
    }
    
    CVBot bot(init_point);
    double precision = 0.0;
    
    // Read images of the objects
    ReadObjectImages();
    LoadScenes();
    
    // Assign Initial coordinates for Santa, Fastener, Cannon
    cv::Point santa_coords;
    cv::Point cannon_coords(15, 282);
    cv::Point fastener_coords(33, 329);
    
    InitVals init_vals;
    cv::Mat scene_image;
    int curr_found_num = 0;
    for (int i = start_index; i < scenes.size(); ++i)
    {
        scene_image = cv::imread(dir_scenes + scenes[i].image_name);
        try
        {
            std::cout << "[" << i << "] Scene: " << scenes[i].image_name << "\n";
            init_vals = bot.findInitialValues(scene_image);

            CV_Assert(init_vals.velocity > 0);
            CV_Assert(init_vals.angle >= 0 && init_vals.angle < 90.0);
        }
        catch (...)
        {
            std::cerr << "Error while invoking Bot's functions or initial values exceeded the limits!";
            continue;
        }
        
        double init_angle_rad = init_vals.angle * CV_PI / 180;
        int dx = cvRound(init_vals.velocity * cos(init_angle_rad));

        int x = cvRound(init_point.x + 82 * cos(init_angle_rad)); // intial x coordinate of Santa
        int y;
        
        std::vector<cv::Mat> rotated_objects; // images of target objects with rotation
        cv::Mat drawn_image;
        if (visualize)
        {
            objects[obj_type::SCENE].copyTo(drawn_image);
            // Do needed transformations on objects and save them in a vector
            for (int m = 0; m < scenes[i].targets.size(); ++m)
            {
                double R = sqrt(pow(scenes[i].targets[m].second.size.width, 2) +
                                pow(scenes[i].targets[m].second.size.height, 2)) / 2;
                cv::Mat object;
                cv::copyMakeBorder
                (
                 objects[scenes[i].targets[m].first],
                 object,
                 R - objects[scenes[i].targets[m].first].rows / 2,
                 R - objects[scenes[i].targets[m].first].rows / 2,
                 R - objects[scenes[i].targets[m].first].cols / 2,
                 R - objects[scenes[i].targets[m].first].cols / 2,
                 cv::BORDER_CONSTANT,
                 cv::Scalar(0, 0, 0, 0)
                 );
                rotate(object, -scenes[i].targets[m].second.angle);
                rotated_objects.push_back(object);
            }
            
            // Draw canes if any
            for (int m = 0; m < scenes[i].canes.size(); ++m)
            {
                double R = sqrt(pow(scenes[i].canes[m].second.size.width / 2, 2) +
                                pow(scenes[i].canes[m].second.size.height / 2, 2));
                cv::Mat object;
                cv::copyMakeBorder
                (
                 objects[scenes[i].canes[m].first], object,
                 R - objects[scenes[i].canes[m].first].rows / 2,
                 R - objects[scenes[i].canes[m].first].rows / 2,
                 R - objects[scenes[i].canes[m].first].cols / 2,
                 R - objects[scenes[i].canes[m].first].cols / 2,
                 cv::BORDER_CONSTANT,
                 cv::Scalar(0, 0, 0, 0)
                 );
                rotate(object, -scenes[i].canes[m].second.angle);
                overlayImage
                (
                 drawn_image,
                 object,
                 cv::Point2i(scenes[i].canes[m].second.center.x - object.cols / 2,
                             scenes[i].canes[m].second.center.y - object.rows / 2)
                 );
            }
            
            // Drawing image of the cannon
            cv::Mat cannon;
            cv::copyMakeBorder(objects[obj_type::CANNON], cannon, 30, 0, 0, 11, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0, 0));
            cv::Point2f pt(35, 64);
            cv::Mat r = cv::getRotationMatrix2D(pt, init_vals.angle, 1.0);
            cv::warpAffine(cannon, cannon, r, cannon.size());
            overlayImage(drawn_image, cannon, cannon_coords);
            
            // Drawing image of the fastener
            overlayImage(drawn_image, objects[obj_type::FASTENER], fastener_coords);
        }
        
        int target_num = scenes[i].targets.size();
        while (true)
        {
            y = cvRound(init_point.y - (x - init_point.x) * tan(init_angle_rad) + (x - init_point.x) * (x - init_point.x) * g /
                        (2 * pow(init_vals.velocity, 2) *  pow(cos(init_angle_rad), 2)));
            
            // Radius of a SANTA
            double r = sqrt(pow(objects[obj_type::SANTA].cols/2, 2) + pow(objects[obj_type::SANTA].rows/2, 2));
            // Find collision between Santa and objects
            for (int l = 0; l < scenes[i].targets.size(); ++l)
            {
                // Radius of target object
                double R = sqrt(pow(scenes[i].targets[l].second.size.width, 2) +
                                pow(scenes[i].targets[l].second.size.height, 2)) / 2;
                
                // Distance between centers of two circles
                double distance = sqrt(pow(scenes[i].targets[l].second.center.x - x, 2) +
                                       pow(scenes[i].targets[l].second.center.y - y, 2));
                if (distance < r + R)
                {
                    scenes[i].targets.erase(scenes[i].targets.begin() + l);
                    if (visualize)
                    {
                        rotated_objects.erase(rotated_objects.begin() + l);
                    }
                    
                    ++curr_found_num;
                }
            }
            
            // If visualize is turned on then show the animation
            if (visualize && (x - init_point.x) % dx == 0)
            {
                std::string win_name = "Scene " + std::to_string(i);
                drawn_image.copyTo(scene_image);
                
                // Draw targets
                for (int m = 0; m < rotated_objects.size(); ++m)
                {
                    overlayImage
                    (
                     scene_image,
                     rotated_objects[m],
                     cv::Point(scenes[i].targets[m].second.center.x - rotated_objects[m].cols / 2,
                               scenes[i].targets[m].second.center.y - rotated_objects[m].rows / 2)
                     );
                }
                
                // Drawing image of the Santa
                cv::Mat santa;
                double R = sqrt(pow(objects[obj_type::SANTA].cols, 2) +
                                pow(objects[obj_type::SANTA].rows, 2)) / 2;
                
                cv::copyMakeBorder
                (
                 objects[obj_type::SANTA],
                 santa,
                 R - objects[obj_type::SANTA].rows / 2,
                 R - objects[obj_type::SANTA].rows / 2,
                 R - objects[obj_type::SANTA].cols / 2,
                 R - objects[obj_type::SANTA].cols / 2,
                 cv::BORDER_CONSTANT,
                 cv::Scalar(0, 0, 0, 0)
                );
                double curr_angle = atan(tan(init_angle_rad) - (x - init_point.x) * g /
                                         (pow(init_vals.velocity, 2) * pow(cos(init_angle_rad), 2))) * 180 / CV_PI;
                rotate(santa, curr_angle);
                santa_coords = cv::Point(cvRound(x - R), cvRound(y - R));
                cv::circle(scene_image, cv::Point(santa_coords.x + R, santa_coords.y + R), R, cv::Scalar(252, 243, 236), -1);
                overlayImage(scene_image, santa, santa_coords);
                
                cv::imshow(win_name, scene_image);
                cv::waitKey(animation_speed);
            }   
            
            ++x;
            
            if (y > 390 || x >= 899) // Santa falls down or passed all the scene, stop it!
            {
                precision += (double)curr_found_num / target_num;
                curr_found_num = 0;
                if (visualize)
                {
                    cv::waitKey(250 * animation_speed / 100);
                    cv::destroyAllWindows();
                }
                
                break;
            }
            
            if (y < 0) // If Santa goes higher just skip this mmoment
            {
                int half_x = cvFloor(tan(init_angle_rad) / (2 * g / (2 * pow(init_vals.velocity, 2) *  pow(cos(init_angle_rad), 2))));
                int dist2half = half_x - (x - init_point.x); // distance from current x to the middle of the flight
                if (dist2half < 0)
                {
                    ++x;
                }
                else
                {
                    x += 2 * dist2half;
                }
            }
        }
    }
    
    std::cout << "\nTests are finished.\n";
    std::cout << "Accuracy of your algorithm is: " << 100 * precision / (100 - start_index) << "% (Number of images: " << 100 - start_index << ").\n";
}

void Game::LoadScenes()
{
	std::ifstream infile(dir_scenes + "metadata.txt");
	
	if (!infile.is_open())
	{
		printf("Coudn't read file \"%s\"!", (dir_scenes + "metadata.txt").c_str());
		exit(EXIT_FAILURE);
	}
	
	std::string line;
	while (std::getline(infile, line))
	{
		Scene new_scene;
		new_scene.ReadFromString(line);
		scenes.push_back(new_scene);
	}

	infile.close();
    
    return;
}

void Scene::ReadFromString(const std::string &line)
{
	size_t pos1 = 0;
	size_t pos2 = line.find_first_of(';', pos1);

	image_name = line.substr(pos1, pos2 - pos1);
	pos2 = line.find_first_of(";", pos2 + 1);
	while (true)
	{
		pos1 = pos2 + 1; // was altered
		if (pos1 == line.length())
		{
			break;
		}

		pos2 = line.find_first_of(":", pos1);
		obj_type objType = (obj_type)std::stoi(line.substr(pos1, pos2 - pos1));
		cv::RotatedRect rotRect;
		pos1 = pos2 + 2;
		pos2 = line.find_first_of(";", pos1);
		std::string token = line.substr(pos1, pos2 - pos1);
		size_t pos = pos1;
		for (int k = 0; k < 5; ++k)
		{
			token = line.substr(pos, line.find(' ') - pos);
			if (k == 0)
			{
				rotRect.center.x = atof(token.c_str());
			}
			else if (k == 1)
			{
				rotRect.center.y = atof(token.c_str());
			}
			else if (k == 2)
			{
				rotRect.size.width = atof(token.c_str());
			}
			else if (k == 3)
			{
				rotRect.size.height = atof(token.c_str());
			}
			else
			{
				rotRect.angle = atof(token.c_str());
			}

			pos = line.find(' ', pos) + 1;
		}

		std::pair<obj_type, cv::RotatedRect> targetOrCane(objType, rotRect);
		if (objType > SCENE && objType < CANE_RED_S)
		{
			targets.push_back(targetOrCane);
		}
		else
		{
			canes.push_back(targetOrCane);
		}
	}

	return;
}

void Game::ReadObjectImages()
{
	objects.push_back(cv::imread(dir_resources + "santa.png", CV_LOAD_IMAGE_UNCHANGED));
	objects.push_back(cv::imread(dir_resources + "cannon.png", CV_LOAD_IMAGE_UNCHANGED));
	objects.push_back(cv::imread(dir_resources + "fastener.png", CV_LOAD_IMAGE_UNCHANGED));
	objects.push_back(cv::imread(dir_resources + "scene.png", CV_LOAD_IMAGE_UNCHANGED));
    
	objects.push_back(cv::imread(dir_resources + "ball_red_s.png", CV_LOAD_IMAGE_UNCHANGED));
	objects.push_back(cv::imread(dir_resources + "ball_red_l.png", CV_LOAD_IMAGE_UNCHANGED));
	objects.push_back(cv::imread(dir_resources + "ball_green_s.png", CV_LOAD_IMAGE_UNCHANGED));
	objects.push_back(cv::imread(dir_resources + "ball_green_l.png", CV_LOAD_IMAGE_UNCHANGED));
	objects.push_back(cv::imread(dir_resources + "ball_sky_s.png", CV_LOAD_IMAGE_UNCHANGED));
	objects.push_back(cv::imread(dir_resources + "ball_sky_l.png", CV_LOAD_IMAGE_UNCHANGED));
    
	objects.push_back(cv::imread(dir_resources + "box_red_s.png", CV_LOAD_IMAGE_UNCHANGED));
	objects.push_back(cv::imread(dir_resources + "box_red_l.png", CV_LOAD_IMAGE_UNCHANGED));
	objects.push_back(cv::imread(dir_resources + "box_green_s.png", CV_LOAD_IMAGE_UNCHANGED));
	objects.push_back(cv::imread(dir_resources + "box_green_l.png", CV_LOAD_IMAGE_UNCHANGED));
	objects.push_back(cv::imread(dir_resources + "box_blue_s.png", CV_LOAD_IMAGE_UNCHANGED));
	objects.push_back(cv::imread(dir_resources + "box_blue_l.png", CV_LOAD_IMAGE_UNCHANGED));
    
	objects.push_back(cv::imread(dir_resources + "cane_red_s.png", CV_LOAD_IMAGE_UNCHANGED));
	objects.push_back(cv::imread(dir_resources + "cane_red_l.png", CV_LOAD_IMAGE_UNCHANGED));
	objects.push_back(cv::imread(dir_resources + "cane_green_s.png", CV_LOAD_IMAGE_UNCHANGED));
	objects.push_back(cv::imread(dir_resources + "cane_green_l.png", CV_LOAD_IMAGE_UNCHANGED));
	objects.push_back(cv::imread(dir_resources + "cane_blue_s.png", CV_LOAD_IMAGE_UNCHANGED));
	objects.push_back(cv::imread(dir_resources + "cane_blue_l.png", CV_LOAD_IMAGE_UNCHANGED));
    
    return;
}

// Rotate src matrix by angle degrees using warpAffine
void Game::rotate(cv::Mat &src, const double &angle)
{
	cv::Point2f p(src.cols / 2, src.rows / 2);
	cv::Mat r = cv::getRotationMatrix2D(p, angle, 1.0);
	cv::warpAffine(src, src, r, cv::Size(src.cols, src.rows));

	return;
}

void Game::overlayImage(cv::Mat background, cv::Mat foreground, const cv::Point2i &tl)
{
	cv::Mat subRegion1;
	cv::Mat subRegion2;
	cv::Rect region = cv::Rect(cv::Point2i(0, 0), background.size()) & cv::Rect(tl, foreground.size());
	if (region.area() != 0)
	{
		subRegion1 = background(region);
		region.x = abs(tl.x - region.x);
		region.y = abs(tl.y - region.y);
		subRegion2 = foreground(region);
	}
	else
	{
//		printf("Error: No overlay!\n");
		return;
	}

	cv::MatIterator_<cv::Vec4b> it, it2, end, end2;
	for (it = subRegion1.begin<cv::Vec4b>(), end = subRegion1.end<cv::Vec4b>(),
		it2 = subRegion2.begin<cv::Vec4b>(), end2 = subRegion2.end<cv::Vec4b>(); it != end; ++it, ++it2)
	{
		double opacity = ((double)(*it2)[3]) / 255;

		if (opacity > 0)
		{
			(*it)[0] = (*it)[0] * (1 - opacity) + (*it2)[0] * opacity;
			(*it)[1] = (*it)[1] * (1 - opacity) + (*it2)[1] * opacity;
			(*it)[2] = (*it)[2] * (1 - opacity) + (*it2)[2] * opacity;
		}
	}
}