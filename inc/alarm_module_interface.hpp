/**
 * @author Alejandro Solozabal
 *
 * @file alarm_module_interface.hpp
 *
 */

#ifndef IALARM_MODULE__H_
#define IALARM_MODULE__H_

/*******************************************************************
 * Struct declaration
 *******************************************************************/
struct AlarmModuleConfig
{
    virtual ~AlarmModuleConfig() = default;
};

/*******************************************************************
 * Class declaration
 *******************************************************************/
class IAlarmModule
{
public:
    /**
     * @brief Destructor
     * 
     */
    virtual ~IAlarmModule() {};

    /**
     * @brief Start the module
     * 
     * @return 0 if ok
     */
    virtual int Start() = 0;

    /**
     * @brief Stop the module
     * 
     * @return 0 if ok
     */
    virtual int Stop() = 0;

    /**
     * @brief Check if the module is runnning
     * 
     * @return true if it is running
     */
    virtual bool IsRunning() = 0;

    /**
     * @brief Update module's config
     * 
     */
    virtual void UpdateConfig(AlarmModuleConfig& config) = 0;

};

#endif /* IALARM_MODULE__H_ */
